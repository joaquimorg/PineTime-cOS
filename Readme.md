<pre>
 ######                  #######                           #######  #####  
 #     # # #    # ######    #    # #    # ######     ####  #     # #     # 
 #     # # ##   # #         #    # ##  ## #         #    # #     # #       
 ######  # # #  # #####     #    # # ## # #####     #      #     #  #####  
 #       # #  # # #         #    # #    # #         #      #     #       # 
 #       # #   ## #         #    # #    # #         #    # #     # #     # 
 #       # #    # ######    #    # #    # ######     ####  #######  #####  

 PineTime-cOS
</pre>

PineTime-cOS 
========================================


<pre>
$ git clone --recurse-submodules https://github.com/joaquimorg/PineTime-cOS.git
</pre>


arm-none-eabi-gdb.exe -ex="target extended-remote 192.168.1.20:3333" .\_build\pinetime-cos.out

b lv_log.c:69

b nrf_log_frontend.c:602
