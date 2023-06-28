/***************************************************************************
*    Copyright (c) 2013, Broadcom Corporation
*    All rights reserved.
*
*  Statement regarding contribution of copyrighted materials to VESA:
*
*  This code is owned by Broadcom Corporation and is contributed to VESA
*  for inclusion and use in its VESA Display Stream Compression specification.
*  Accordingly, VESA is hereby granted a worldwide, perpetual, non-exclusive
*  license to revise, modify and create derivative works to this code and
*  VESA shall own all right, title and interest in and to any derivative 
*  works authored by VESA.
*
*  Terms and Conditions
*
*  Without limiting the foregoing, you agree that your use
*  of this software program does not convey any rights to you in any of
*  Broadcom�s patent and other intellectual property, and you
*  acknowledge that your use of this software may require that
*  you separately obtain patent or other intellectual property
*  rights from Broadcom or third parties.
*
*  Except as expressly set forth in a separate written license agreement
*  between you and Broadcom, if applicable:
*
*  1. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
*  REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
*  OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
*  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
*  NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
*  ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
*  CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
*  OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
*  BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
*  SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
*  IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
*  ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
*  OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
*  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/

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


#define AVE_VAR_LEN 30	//number of consecutive pixels used to calculate the average/var/standard
#define ST_DIV 		3   //weight of standard, which is used to modified psnr 
#define CNT_DIV		100 //adjust according to picture's w&h

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

	/***modified***/
	int    i,j;
	int    counter = 0;
    double pxl_err_reg[AVE_VAR_LEN][3] = {0}; //recent 9 pixels
	double temp 	= 0;
	double sum[3] 	   = {0,0,0};
	double var[3] 	   = {0,0,0};
	double average[3]  = {0,0,0};
	double standard[3] = {0,0,0};
	double mse_plus_var= 0.0;
	double sumSqrError_mod_plus_var = 0.0;
	double sumStandard = 0;
	double sumVar = 0;

	double m_psnr_var = 0.0;
	double m_psnr_count = 0.0;



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
					

				/*************************************modified****************************************/
					pxl_err_reg[AVE_VAR_LEN-1][ch] = abs(err);	//reg[9] store newest pixel
					sum[ch] += abs(err)-pxl_err_reg[0][ch]; //sum = sum + newest -oldest

					if(abs(err)>10) counter++; //count the num of condition (diff of pic_i & pic_o)

				}//pixel end

				for(i=0;i<AVE_VAR_LEN-1;i++)//10 shift_reg store recent 10 pxl error
				{
					for(j=0;j<3;j++)
					{
						pxl_err_reg[i][j] = pxl_err_reg[i+1][j];
					}
				}
				
				//calculate mean and variance of consecutive 10 pixel err

				for(ch=0;ch<3;ch++)
				{
					//for(i=0;i<AVE_VAR_LEN;i++)//cal 10 pixel sum
					//{
					//	sum[ch] += pxl_err_reg[ch][i];
					//}

					int ave_num = (xcnt<AVE_VAR_LEN) ? (xcnt + 1) : AVE_VAR_LEN;//num used to divide sum, if xcnt < 9, ave_num = xcnt, otherwise it equals AVE_VAR_LEN;
					average[ch] = sum[ch]/ave_num; //cal consecutive ave_num pixel average, note: if len=8, ave=sum>>3;

					//print waring message to log
					if(average[ch]>10){
						fprintf(logfp, "Warning: average error of %d pixels is more than 10, average = %6.2f \n", ave_num, average);
						fprintf(logfp, "the waring position: hpos: %d, vpos: %d \n\n", xcnt, ycnt);
					}

					for(i=0;i<AVE_VAR_LEN;i++) //cal 10 pixel var & stantard
					{
						temp = pxl_err_reg[i][ch]-average[ch];

						var[ch] += pow(temp,2)/AVE_VAR_LEN;

						standard[ch] = pow(var[ch],0.5);
					}
					sumVar += var[ch]/ST_DIV; // +var, pow2 similar to err
					//sumStandard += standard[ch]/ST_DIV; // stantard is small (in general less than 1), so ST_DIV can be a larger number

					//sum[ch] = 0;//do not need reset because "sum += new-old" above
					var[ch] = 0;//everytime after cal var & average, need reset them in case accumulation unitl overflow

				}//pixel end
			}//line end

			for(j=0;j<3;j++)//set  to 0 at the beginning of a line
			{
				for(i=0;i<AVE_VAR_LEN;i++) pxl_err_reg[i][j] = 0;
				sum[j] = 0;
			}
		}//pic end

		sumSqrError_mod_plus_var = sumSqrError + sumVar;
		//sumSqrError_mod_plus_var = sumSqrError+ sumStandard;

		if (sumSqrError != 0) {
			mse_plus_var= sumSqrError_mod_plus_var / ((double) (p_in->h * p_in->w * 3)); //3 cpnt
			mse = sumSqrError / ((double) (p_in->h * p_in->w * 3)); //3 cpnt

			m_psnr_var = 10.0 * log10( (double)Max*Max / mse_plus_var );
			m_psnr_count = 10.0 * log10( (double)Max*Max / mse ) - counter/CNT_DIV;
			psnr = 10.0 * log10( (double)Max*Max / mse );

			fprintf(logfp, "PSNR over RGB channels = %6.2f  \n", psnr);
			fprintf(logfp, "PSNR modified( var ) = %6.2f  \n", m_psnr_var);
			fprintf(logfp, "PSNR modified(count) = %6.2f  \n", m_psnr_count);


		} else {
			fprintf(logfp, "PSNR over RGB channels = Inf   \n");
		}
		fprintf(logfp, "Max{|error|} = %4d   (R =%4d, G =%4d, B =%4d)   \n", MAX(maxErrR, MAX(maxErrG, maxErrB)), maxErrR, maxErrG, maxErrB);

		int amount = counter>>4;
		double m_psnr_count = psnr - amount;
		fprintf(logfp,"counter = %d \n", counter);




/*********************************************************************************************/
	} else {//yuv

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
}

