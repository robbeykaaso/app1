* config  
    - test_server  
        if it is true, software will use the integrated server, otherwise software will use the remoted server  
    - local_fs  
        if it is true, software will use the filesystem, otherwise software will try to use minIO. if the minIO folder is existed in the current work directory, software will use the local minIO service, otherwise it will use the remote minIO service  
    - fs_root  
        it defines the root path of filesystem for `local_fs`, the default is "deepsight"  
    - aws_fs  
        it defines the config for the remote minIO service  
</br>

* command line parameters  
    - -d: run the "unitTest" pipeline  
    - -m GUI: start the software by demo mode  
    - -md: start the software by engineer mode  