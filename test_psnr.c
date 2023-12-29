#include <stdio.h>
#include <string.h>
#include "psnr.h"
#include "utl.h"

#define WIN32

int main()
{
    printf("\n\n\n---------------\nthe main function is running...\n\n");

    char infname1[1024] = "test_face.ppm";
    //char infname2[1024] = "test_face.ppm";
    //char infname2[1024] = "test_face_short_bar.ppm";
    //char infname2[1024] = "test_face_bar_h.ppm";
    //char infname2[1024] = "test_face_bar_v.ppm";
    //char infname2[1024] = "test_face_long_bar.ppm";
    //char infname2[1024] = "test_face_gauss.ppm";
    //char infname2[1024] = "test_face_gauss1.ppm";
    char infname2[1024] = "test_face_gauss2.ppm";
    //char infname2[1024] = "t_Barbara_bar.ppm";
    //char infname2[1024] = "test_face_salt&pepper1.ppm";
    //char infname2[1024] = "test_face_salt&pepper2.ppm";

    char fn_log[1024] = "log.txt";

    printf("inf1 = %s\ninf2 = %s\nfn_log = %s\n", infname1, infname2, fn_log);

    pic_t *ip1=NULL, *ip2=NULL;
    FILE* logfp = NULL;

    ppm_read(infname1, &ip1); //fclose in ppm_read, no need to close here
    ppm_read(infname2, &ip2);



    if( NULL == (logfp=fopen(fn_log, "w") ) )
    {
        printf("log open failed\n");
        exit(1);
    }
        
    compute_and_display_PSNR(ip1, ip2, 8, logfp);

    fclose(logfp);

    return 0;
}