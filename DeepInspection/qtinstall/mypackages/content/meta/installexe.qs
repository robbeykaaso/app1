/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

var Dir = new function () {
    this.toNativeSparator = function (path) {
        if (systemInfo.productType === "windows")
            return path.replace(/\//g, '\\');
        return path;
    }
};

var storage_dir = "C:/ProgramData";

function Component()
{
    // default constructor
    if (installer.isInstaller()) {
        component.loaded.connect(this, Component.prototype.installerLoaded);
        //installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    }
}

Component.prototype.installerLoaded = function () {
    if (installer.isInstaller()) {
        if (installer.addWizardPage(component, "MyPage", QInstaller.ReadyForInstallation)) {
            var widget = gui.pageWidgetByObjectName("DynamicMyPage");
            if (widget != null) {
                widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);
                widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);

                widget.windowTitle = "Storage Folder";
                widget.targetDirectory.text = Dir.toNativeSparator(storage_dir);
            }
        }
    }
}

Component.prototype.targetChanged = function (text) {
    var widget = gui.pageWidgetByObjectName("DynamicMyPage");
    if (widget != null) {
        if (text != "") {
            if (installer.fileExists(text)) {
                widget.complete = true;
                storage_dir = text;
                return;
            }
        }
        widget.complete = false;
    }
}

Component.prototype.chooseTarget = function () {
    var widget = gui.pageWidgetByObjectName("DynamicMyPage");
    if (widget != null) {
        var newTarget = QFileDialog.getExistingDirectory("Choose your target directory.", widget.targetDirectory.text);
        if (newTarget != "")
            widget.targetDirectory.text = Dir.toNativeSparator(newTarget);
    }
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/DeepInspection.exe", "@StartMenuDir@/DeepInspection.lnk",
            "workingDirectory=@TargetDir@");
        component.addOperation("CreateShortcut", "@TargetDir@/DeepInspection.exe", "@DesktopDir@/DeepInspection.lnk",
            "workingDirectory=@TargetDir@");
        //component.addOperation("Mkdir", "C:/hellohello");
            //https://www.cnblogs.com/oloroso/p/6775318.html#%E6%93%8D%E4%BD%9C-operations
        component.addOperation("Execute", "@TargetDir@/minIO/gen.bat", storage_dir, "@TargetDir@/minIO/");
        component.addOperation("Execute", "@TargetDir@/minIO/storage_install.bat");
    }
}