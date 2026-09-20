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

const NOT_INSTALLED_MESSAGE =
    "Axel-GUI is not installed. Please install axel-gui for the extension to work.\n\n" +
    "If you have uninstalled axel-gui, please also remove the Axel GUI Integration extension.";

function isLocalOrRouterAddress(hostname) {
    if (hostname === "localhost" || hostname.endsWith(".local")) return true;
    const privateIpRegex = /^(127\.|10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[0-1])\.)/;
    return privateIpRegex.test(hostname);
}

async function showNotInstalledAlert() {
    try {
        browser.notifications.create({
            type: "basic",
            title: "Axel-GUI Not Installed",
            message: "Axel-GUI is not installed. Please install axel-gui for the extension to work, or remove this extension."
        });
    } catch (e) { }
    try {
        const tabs = await browser.tabs.query({ active: true, currentWindow: true });
        if (tabs && tabs[0] && tabs[0].id) {
            await browser.tabs.executeScript(tabs[0].id, {
                code: `alert(${JSON.stringify(NOT_INSTALLED_MESSAGE)});`
            });
            return;
        }
    } catch (e) { }
    try {
        const popupHtml = `
            <!DOCTYPE html>
            <html>
            <head>
                <meta charset="utf-8">
                <title>Axel-GUI Required</title>
                <style>
                    body { font-family: sans-serif; background: #282828; color: #ebdbb2; padding: 20px; text-align: center; }
                    h3 { color: #fb4934; margin-top: 0; }
                    p { font-size: 13px; line-height: 1.6; }
                    button { background: #fe8019; color: #282828; border: none; padding: 8px 20px; border-radius: 4px; font-weight: bold; cursor: pointer; margin-top: 10px; }
                    button:hover { background: #fabd2f; }
                </style>
            </head>
            <body>
                <h3>Axel-GUI Not Installed</h3>
                <p>Axel-GUI is not installed. Please install <b>axel-gui</b> for the extension to work.<br><br>
                   If you have uninstalled axel-gui, please also remove the <i>Axel GUI Integration</i> extension.</p>
                <button onclick="window.close()">OK</button>
            </body>
            </html>
        `;
        browser.windows.create({
            url: "data:text/html," + encodeURIComponent(popupHtml),
            type: "popup",
            width: 460,
            height: 250
        });
    } catch (e) { }
}

browser.downloads.onCreated.addListener(async (downloadItem) => {
    const rawUrl = downloadItem.url;
    if (!rawUrl || rawUrl.startsWith("blob:") || rawUrl.startsWith("data:") || rawUrl.startsWith("about:")) {
        return;
    }

    try {
        if (browser.captivePortal && browser.captivePortal.getState) {
            const portalState = await browser.captivePortal.getState();
            if (portalState === "locked_portal") return;
        }
    } catch (e) { }

    let parsedUrl;
    try {
        parsedUrl = new URL(rawUrl);
    } catch (e) {
        return;
    }

    if (CAPTIVE_PORTAL_DOMAINS.includes(parsedUrl.hostname.toLowerCase())) return;
    if (isLocalOrRouterAddress(parsedUrl.hostname)) return;

    const pathname = parsedUrl.pathname.toLowerCase();
    if (pathname.includes("/login") || pathname.includes("/portal") || pathname.includes("/guest") || pathname.includes("/hotspot")) {
        return;
    }

    for (const ext of IGNORED_EXTENSIONS) {
        if (pathname.endsWith(ext)) return;
    }

    if (downloadItem.mime && IGNORED_MIME_TYPES.includes(downloadItem.mime.toLowerCase())) {
        return;
    }
    let cookieString = "";
    try {
        const cookies = await browser.cookies.getAll({ url: rawUrl });
        cookieString = cookies.map(c => `${c.name}=${c.value}`).join("; ");
    } catch (e) { }
    try {
        const response = await browser.runtime.sendNativeMessage("axel_gui_host", {
            url: rawUrl,
            filename: downloadItem.filename || "",
            cookie: cookieString,
            userAgent: navigator.userAgent,
            referer: downloadItem.referrer || parsedUrl.origin
        });

        if (response && response.error === "not_installed") {
            showNotInstalledAlert();
            return;
        }
        await browser.downloads.cancel(downloadItem.id);
        await browser.downloads.erase({ id: downloadItem.id });

    } catch (err) {
        showNotInstalledAlert();
    }
});
