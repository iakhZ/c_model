/*

230716:
	work:
		1.abandon psnr(var): verify that mpsnr(var) is not necessary, it dosen't work well in codec, because mse is enough
		2.abandon psnr(count): no need to usr error pixel counter because ave counter is enough
		2.psnr(ave): psnr adjusted amount now is related to  100 * sqrt (counter/picture_size), adj amount is clamped in range 0-100
	next:
		1.luma is vision loss factor, take it into consideration
		2.can refer to SSIM, it seems a good method, but process is complex to implement in hardware4         

*/


#ifdef WIN32
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "vdo.h"
#include "psnr.h"
#else
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "vdo.h"
#include "psnr.h"
#endif

#include "dsc_opt.h" //used for optimazing dsc algorithm


/*
	For RGB inputs we will compute the PSNR over all three channels.
	For YCbCr inputs we will only compute the PSNR over the luma channel.
*/


void compute_and_display_PSNR(pic_t *p_in, pic_t *p_out, int bpp, FILE *logfp)
{
	int xcnt, ycnt;
	int Max, ch, wd = 0, ht = 0;
	double sumSqrError = 0.0;
	double psnr = 0.0;
	double mse = 0.0;
	int err = 0;
	int maxErrR = 0;
	int maxErrG = 0;
	int maxErrB = 0;

	printf("\n-----------------------------------\nnow start to compute psnr\n-----------------------------------\n");

#ifdef PSNR_OPT

	int i;
	int counter = 0;	  					//single abnormal pixels counter
	int	count_ave = 0; 	  					//consecutive average err(out of range) counter
	int ave_num = 0; 	  					//number used to divide the average
	int abnormal_pos = 0; 					//record most recent position where consecutive "average>=10" happened 

    double pxl_err_reg[AVE_LEN][3] = {0}; 	//recent AVE_LEN pixels
	double temp 	= 0; 					//x-E(x) used for Var

	double sum[3] 	   = {0,0,0}; 			//recent 10 pixel cpnt err sum
	double var[3] 	   = {0,0,0}; 			//recent 10 pixel cpnt var
	double average[3]  = {0,0,0}; 			//recent 10 pixel cpnt average, equals to "sum/ave_num"

	double sumVar = 0; 						//sum of var

#endif


	Max = (1<<bpp)-1;

	if (p_in->bits != p_out->bits)
		printf("in out bits not matched\n");
	
	if (p_in->color == RGB) 
	{
		wd = p_in->w; 
		for(ycnt=0; ycnt<p_in->h; ycnt++) //column
		{
			for(xcnt=0; xcnt<p_in->w; xcnt++) //line
			{
				for (ch=0; ch<3; ch++) //3 cpnt of 1 pixel
				{
					switch(ch) {
					case 0:
						err = p_out->data.rgb.r[ycnt][xcnt] - p_in->data.rgb.r[ycnt][xcnt];
						if (abs(err) > maxErrR) {
							maxErrR = abs(err);	
							//fprintf(logfp, "R -- x: %d, y: %d, maxErrR = %d\n",xcnt, ycnt, maxErrR);						
						}			
#ifdef GENERATE_ERROR_IMAGE
						p_out->data.rgb.r[ycnt][xcnt] = 512 + err;
#endif										
						break;
					case 1:
						err = p_out->data.rgb.g[ycnt][xcnt] - p_in->data.rgb.g[ycnt][xcnt];
						if (abs(err) > maxErrG) {
							maxErrG = abs(err);	
							//fprintf(logfp, "G -- x: %d, y: %d, maxErrG = %d\n",xcnt, ycnt, maxErrG);							
						}							
#ifdef GENERATE_ERROR_IMAGE
						p_out->data.rgb.g[ycnt][xcnt] = 512 + err;
#endif										
						break;
					case 2:
						err = p_out->data.rgb.b[ycnt][xcnt] - p_in->data.rgb.b[ycnt][xcnt];
						if (abs(err) > maxErrB) {
							maxErrB = abs(err);	
							//fprintf(logfp, "B -- x: %d, y: %d, maxErrB = %d\n",xcnt, ycnt, maxErrB);							
						}							
#ifdef GENERATE_ERROR_IMAGE
						p_out->data.rgb.b[ycnt][xcnt] = 512 + err;
#endif										
						break;
					}

					sumSqrError += (double) err * err;//origin sum qeeor

	
#ifdef PSNR_OPT
					pxl_err_reg[AVE_LEN-1][ch] = abs(err);	//reg[last] store newest pixel
					sum[ch] += abs(err)-pxl_err_reg[0][ch]; //sum = sum + newest -oldest

					if(abs(err)>10) counter++; //count the num of condition (diff of pic_i & pic_o)

				//----------------
					for(i=0;i<AVE_LEN-1;i++)//10 shift reg store recent 10 pxl error
					{
							pxl_err_reg[i][ch] = pxl_err_reg[i+1][ch];
					}
					
					ave_num = (xcnt < AVE_LEN) ? (xcnt + 1) : AVE_LEN;	//num used to divide sum, if xcnt < 9, ave_num = xcnt, otherwise it equals AVE_LEN;
					average[ch] = sum[ch] / ave_num; 				  	//compute consecutive ave_num pixel average, note: if len=4, ave=sum>>2;

					//print waring message to log
					if(average[ch]>AVE_THRE){

						count_ave = (xcnt-abnormal_pos) < DISTANCE ? count_ave + 2 : count_ave + 1; //consecutive abnormal pixel then count_ave++
						abnormal_pos = xcnt; //record last abnomal pixel position

						/*abnormal pixel message print*/
						//printf("abnormal pos: x = %d, y = %d, cpnt = %d, average = %6.2f\n", xcnt, ycnt, ch, average[ch]);
						fprintf(logfp, "abnormal pos: x = %d, y = %d, cpnt = %d, average = %6.2f\n", xcnt, ycnt, ch, average[ch]);

					}

					for(i=0;i<AVE_LEN;i++) //compute 10 pixel var
					{
						temp = pxl_err_reg[i][ch]-average[ch];

						var[ch] += pow(temp,2); //var accumulate

					}

					sumVar += var[ch];
					var[ch] = 0;

				//----------------
#endif	
				}//pixel end

			}//line end


#ifdef PSNR_OPT
			abnormal_pos = 0; //set abnormal pos to 0 every line start

			for(ch=0;ch<3;ch++)//set  to 0 at the beginning of a line
			{
				for(i=0;i<AVE_LEN;i++) 
				{
					pxl_err_reg[i][ch] = 0;
				}
				sum[ch] = 0;
			}
#endif

		}//frame end

		if (sumSqrError != 0) //original psnr
		{
			mse = sumSqrError / ((double) (p_in->h * p_in->w * 3)); //3 cpnt 

			psnr = 10.0 * log10( (double)Max*Max / mse );

			printf("\nsumSqrError: %f\n",sumSqrError);
			printf("Max = %d\nmse = %f \ncount_ave = %d\n\n", Max, mse, count_ave);

			printf("PSNR over RGB channels = %6.2f  \n", psnr);
			fprintf(logfp, "PSNR over RGB channels = %6.2f  \n", psnr);

#ifdef PSNR_OPT

		double sumSqrError_plus_var = sumSqrError + sumVar * VAR_MUL ; //var
		double count_ave_div_pic_size = (double)count_ave/ ((p_in->h * p_in->w)*3);
		double count_ave_div_pic_size_sqrt = sqrt(count_ave_div_pic_size);
		double ave_amount = 100 * count_ave_div_pic_size_sqrt;
		printf("count_ave_div_pic_size_sqrt = %f\n", count_ave_div_pic_size_sqrt);

		//-----------------

		//printf("sum of var(sumVar): %f\n", sumVar);
		//printf("sumSqrError + VAR_MUL * sumVar: %f\n",sumSqrError_plus_var);
		printf("psnr adjust amount: %f\n\n", ave_amount);

		fprintf(logfp,"sumSqrError: %f\n",sumSqrError);
		fprintf(logfp,"sum of var(sumVar): %f\n", sumVar);
		fprintf(logfp,"sumSqrError + VAR_MUL * sumVar: %f\n",sumSqrError_plus_var);
		fprintf(logfp,"psnr adjust amount: %f\n", ave_amount);

		double mse_plus_var= sumSqrError_plus_var / ((double) (p_in->h * p_in->w * 3)); //3 cpnt
		double m_psnr_var_ave = 10.0 * log10( (double)Max*Max / mse_plus_var ) - ave_amount;

		double m_psnr_var = 10.0 * log10( (double)Max*Max / mse_plus_var );
		double m_psnr_ave = 10.0 * log10( (double)Max*Max / mse ) - ave_amount;

		double m_psnr_count = 10.0 * log10( (double)Max*Max / mse ) - (double)(counter>>CNT_SH);//this param should be related to picture size

		//printf("PSNR ( var ) = %6.2f, not necessary, it dosen't work well in codec, because mse is enough\n", m_psnr_var);//feels like it's not very useful
		printf("PSNR ( ave ) = %6.2f\n", m_psnr_ave);
		//printf("PSNR ( var & ave ) = %6.2f  \n", m_psnr_var_ave);
		//printf("PSNR ( count ) = %6.2f,not necessary too\n", m_psnr_count);

		fprintf(logfp, "PSNR ( var ) = %6.2f, not necessary, it dosen't work well in codec, because mse is enough\n", m_psnr_var);
		fprintf(logfp, "PSNR ( ave ) = %6.2f\n", m_psnr_ave);
		//fprintf(logfp, "PSNR ( var & ave ) = %6.2f  \n", m_psnr_var_ave);
		fprintf(logfp, "PSNR ( count ) = %6.2f, not necessary too\n", m_psnr_count);

#endif

		}
		else //no error then psnr = 0
		{
			printf("sumSqrError = 0\n",psnr);///used to debug
			fprintf(logfp, "PSNR over RGB channels = Inf   \n");//problem here
		}

		fprintf(logfp, "Max{|error|} = %4d   (R =%4d, G =%4d, B =%4d)   \n", MAX(maxErrR, MAX(maxErrG, maxErrB)), maxErrR, maxErrG, maxErrB);

	} 
	else //yuv (only consider about y)
	{

		for(ycnt=0; ycnt<p_in->h; ycnt++) 
		{
			for(xcnt=0; xcnt<p_in->w; xcnt++) 
			{
				err = p_out->data.yuv.y[ycnt][xcnt] - p_in->data.yuv.y[ycnt][xcnt];
				sumSqrError += (double)err * err;
			}
		}
		if (sumSqrError != 0) {
			mse = sumSqrError / ((double) (p_in->h * p_in->w));  // no factor of 3, only looking at luma channel
			psnr = 10.0 * log10( (double) Max*Max / mse );
			fprintf(logfp, "PSNR over luma (Y) channel = %6.2f  \n", psnr);
		} 
		else 
			fprintf(logfp, "PSNR over luma (Y) channel = Inf   \n");

		if (p_in->chroma == YUV_422 || p_in->chroma == YUV_420)
			wd = p_in->w/2;
		else if (p_in->chroma == YUV_444)
			wd = p_in->w;
		if (p_in->chroma == YUV_420)
			ht = p_in->h/2;
		else
			ht = p_in->h;

		sumSqrError = 0;
		for(ycnt=0; ycnt<ht; ycnt++) 
		{
			for(xcnt=0; xcnt<wd; xcnt++) 
			{
				err = p_out->data.yuv.u[ycnt][xcnt] - p_in->data.yuv.u[ycnt][xcnt];
				sumSqrError += (double)err * err;
			}
		}
		if (sumSqrError != 0) {
			mse = sumSqrError / ((double) (ht * wd));  // no factor of 3, only looking at luma channel
			psnr = 10.0 * log10( (double) Max*Max / mse );
			fprintf(logfp,"PSNR over chroma (U) channel = %6.2f  \n", psnr);
		} 
		else 
			fprintf(logfp,"PSNR over chroma (U) channel = Inf   \n");
	
		sumSqrError = 0;
		for(ycnt=0; ycnt<ht; ycnt++) 
		{
			for(xcnt=0; xcnt<wd; xcnt++) 
			{
				err = p_out->data.yuv.v[ycnt][xcnt] - p_in->data.yuv.v[ycnt][xcnt];
				sumSqrError += (double)err * err;
			}
		}
		if (sumSqrError != 0) {
			mse = sumSqrError / ((double) (ht * wd));  // no factor of 3, only looking at luma channel
			psnr = 10.0 * log10( (double) Max*Max / mse );
			fprintf(logfp,"PSNR over chroma (V) channel = %6.2f  \n", psnr);
		} 
		else 
			fprintf(logfp,"PSNR over chroma (V) channel = Inf   \n");

	}

	printf("-----------------------------------\n");

}

