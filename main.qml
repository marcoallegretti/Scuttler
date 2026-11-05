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
