const CAPTIVE_PORTAL_DOMAINS = [
    "detectportal.firefox.com",
    "connectivitycheck.gstatic.com",
    "connectivitycheck.android.com",
    "captive.apple.com",
    "msftconnecttest.com",
    "nmcheck.gnome.org",
    "networkcheck.kde.org"
];

const IGNORED_MIME_TYPES = [
    "text/html",
    "application/xhtml+xml",
    "application/xml",
    "text/xml",
    "text/css",
    "text/javascript",
    "application/javascript",
    "application/json"
];

const IGNORED_EXTENSIONS = [
    ".html", ".htm", ".xhtml", ".php", ".asp", ".aspx", ".jsp", ".cgi"
];

function isLocalOrRouterAddress(hostname) {
    if (hostname === "localhost" || hostname.endsWith(".local")) {
        return true;
    }

    const privateIpRegex = /^(127\.|10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[0-1])\.)/;
    return privateIpRegex.test(hostname);
}

browser.downloads.onCreated.addListener(async (downloadItem) => {
    const rawUrl = downloadItem.url;
    if (!rawUrl || rawUrl.startsWith("blob:") || rawUrl.startsWith("data:") || rawUrl.startsWith("about:")) {
        return;
    }

    //check if any unauthenticated captive portal present or not
    try {
        if (browser.captivePortal && browser.captivePortal.getState) {
            const portalState = await browser.captivePortal.getState();
            if (portalState === "locked_portal") {

                return;
            }
        }
    } catch (e) {

    }

    let parsedUrl;
    try {
        parsedUrl = new URL(rawUrl);
    } catch (e) {
        return;
    }


    if (CAPTIVE_PORTAL_DOMAINS.includes(parsedUrl.hostname.toLowerCase())) {
        return;
    }


    if (isLocalOrRouterAddress(parsedUrl.hostname)) {
        return;
    }

    const pathname = parsedUrl.pathname.toLowerCase();
    if (pathname.includes("/login") || pathname.includes("/portal") || pathname.includes("/guest") || pathname.includes("/hotspot")) {
        return;
    }


    for (const ext of IGNORED_EXTENSIONS) {
        if (pathname.endsWith(ext)) {
            return;
        }
    }


    if (downloadItem.mime && IGNORED_MIME_TYPES.includes(downloadItem.mime.toLowerCase())) {
        return;
    }

    // Intercept valid file downloasd
    try {
        await browser.downloads.cancel(downloadItem.id);
        await browser.downloads.erase({ id: downloadItem.id });
    } catch (e) {

    }

    browser.runtime.sendNativeMessage("axel_gui_host", {
        url: rawUrl,
        filename: downloadItem.filename || ""
    });
});
