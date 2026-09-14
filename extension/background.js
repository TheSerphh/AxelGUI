browser.downloads.onCreated.addListener(async (downloadItem) => {
    const url = downloadItem.url;

    // Ignore internal browser schemes or blob URLs if desired
    if (!url || url.startsWith("blob:") || url.startsWith("data:") || url.startsWith("about:")) {
        return;
    }

    try {
        // Cancel and remove the default Firefox download
        await browser.downloads.cancel(downloadItem.id);
        await browser.downloads.erase({ id: downloadItem.id });
    } catch (e) {
        // Download might have completed or was already cancelled
    }

    // Forward the download to the local Axel GUI
    browser.runtime.sendNativeMessage("axel_gui_host", {
        url: url,
        filename: downloadItem.filename || ""
    });
});
