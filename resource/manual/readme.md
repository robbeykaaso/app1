* config  
    - test_server  
        if it is true, software will use the integrated server, otherwise software will use the remote server  
    - local_fs  
        if it is true, software will use the filesystem, otherwise software will try to use minIO. if the minIO folder is existed in the current work directory, software will use the local minIO service, otherwise software will use the remote minIO service  
    - fs_root  
        it defines the root directory of filesystem for `local_fs`, the default is "deepsight"  
    - aws_fs  
        it defines the config for the remote minIO service  
</br>

* command line parameters  
    - -d: run the "unitTest" pipeline  
    - -m GUI: start the software by demo mode  
    - -md: start the software by engineer mode  

* (link)[https://docs.google.com/document/d/1JOe9oP9cqXy2dsT74yxIzFMnMRENll_8Aa-BqERd4IQ/edit?usp=sharing]