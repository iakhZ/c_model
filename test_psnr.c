#include <stdio.h>
#include <string.h>
#include "psnr.h"
#include "utl.h"

#define WIN32

int main()
{
    printf("the main function is running...\n");

    char infname1[1024] = "t_Barbara_jpg.ppm";

    char infname2[1024] = "t_Barbara_noise.ppm";
    char fn_log[1024] = "log.txt";

    printf("inf1 = %s\ninf2 = %s\nfn_log = %s\n", infname1, infname2, fn_log);

    pic_t *ip1=NULL, *ip2=NULL;
    FILE *logfp=NULL;

    ppm_read(infname1, &ip1);
    ppm_read(infname2, &ip2);

    if( NULL == (logfp=fopen(fn_log, "wt") ) )
    {
        printf("log open failed\n");
        exit(1);
    }
        
    compute_and_display_PSNR(ip1, ip2, 24, logfp);

    //fprintf(logfp,"write"); //error: Segmentation fault don't know why
    
    fclose(ip1);//error: Segmentation fault
    fclose(ip2);
    printf("close ip1 and ip2\n");
    fclose(logfp);

    return 0;
}