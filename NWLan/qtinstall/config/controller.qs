function Controller(){
    if (installer.isUninstaller()){
        //installer.finishedCalculateComponentsToUninstall.connect(this, this.uninstallationStarted);
        //installer.performOperation("Execute", ["@TargetDir@/minIO/storage_stop.bat"]);
        //installer.uninstallationStarted.connect(this, this.uninstallationStarted);
        //installer.execute("mkdir", "hellohello2")
    }else{
        //installer.installationFinished.connect(this, this.installationFinished);
        //installer.execute("mkdir", "hellohello")
    }    
}

Controller.prototype.uninstallationStarted = function()
{
    //if (fileExists("@TargetDir@/minIO/storage_uninstall.bat"))
    //    installer.performOperation("Execute", ["@TargetDir@/minIO/storage_uninstall.bat"]);
}

Controller.prototype.installationFinished = function()
{
    //installer.performOperation("Execute", ["@TargetDir@/key.bat"]);
    //installer.performOperation("Execute", ["@TargetDir@/minIO/minIO_netsh.bat"]);
}