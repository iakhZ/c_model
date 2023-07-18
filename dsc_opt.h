
#define	PSNR_OPT
#define MMAP_OPT


//psnr macros
#define VAR_MUL		2	//used to divide var sum 
#define DISTANCE	3	//used to determine the distance between two consecutive abnormal pixel
#define AVE_LEN 	3	//number of consecutive pixels used to calculate the average/var/standard
#define CNT_SH		7   //single abnormal pixels count, shift then add to psnr (adjust according to picture's w&h?)
#define AVE_THRE	30	//average value threshold