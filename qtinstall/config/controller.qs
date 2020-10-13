function Controller(){
    if (installer.isUninstaller()){
        //installer.finishedCalculateComponentsToUninstall.connect(this, this.uninstallationStarted);
        installer.performOperation("Execute", ["@TargetDir@/minIO/storage_stop.bat"]);
        installer.uninstallationStarted.connect(this, this.uninstallationStarted);
        //installer.execute("mkdir", "hellohello2")
    }else{
        //installer.execute("mkdir", "hellohello")
        
    }    
}

Controller.prototype.uninstallationStarted = function()
{
    if (fileExists("@TargetDir@/minIO/storage_uninstall.bat"))
        installer.performOperation("Execute", ["@TargetDir@/minIO/storage_uninstall.bat"]);
}