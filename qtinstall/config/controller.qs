function Controller(){
    if (installer.isUninstaller()){
        installer.uninstallationStarted.connect(this, this.uninstallationStarted);
        //installer.execute("mkdir", "hellohello2")
    }else{
        //installer.execute("mkdir", "hellohello")
        
    }    
}

Controller.prototype.uninstallationStarted = function()
{
    installer.performOperation("Execute", ["@TargetDir@/minIO/storage_stop.bat"]);
    installer.performOperation("Execute", ["@TargetDir@/minIO/storage_uninstall.bat"]);
}