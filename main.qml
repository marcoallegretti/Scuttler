// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: BSD-3-Clause AND EUPL-1.2

import QtQuick
import QtQuick.Controls
import QtWebEngine

ApplicationWindow {
    id: window

    required property url initialUrl

    WebEngineView {
        id: webView

        anchors.fill: parent
        url: window.initialUrl

        onLoadingChanged: function(loadingInfo) {
            if (loadingInfo.status === WebEngineLoadingInfo.LoadStartedStatus) {
                retry.stop()
            } else if (loadingInfo.status === WebEngineLoadingInfo.LoadFailedStatus) {
                console.warn(loadingInfo.errorString)
                if (loadingInfo.errorDomain === WebEngineLoadingInfo.ConnectionErrorDomain)
                    retry.restart()
            }
        }

        onRenderProcessTerminated: function(terminationStatus, exitCode) {
            console.warn("Web content process terminated, status", terminationStatus, "exit code", exitCode)
            retry.restart()
        }
    }

    Timer {
        id: retry

        interval: 1000
        onTriggered: webView.reload()
    }

    ProgressBar {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 5
        background: Item {}
        visible: webView.loading
        from: 0
        to: 100
        value: webView.loadProgress
    }
}
