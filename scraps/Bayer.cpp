/*****************************************************************************
*                                                                            *
*  PrimeSense Sensor 5.0 Alpha                                               *
*  Copyright (C) 2010 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PrimeSense Common.                                   *
*                                                                            *
*  PrimeSense Sensor is free software: you can redistribute it and/or modify *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  PrimeSense Sensor is distributed in the hope that it will be useful,      *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU Lesser General Public License  *
*  along with PrimeSense Sensor. If not, see <http://www.gnu.org/licenses/>. *
*                                                                            *
*****************************************************************************/


/*****************************************************************************
 * Edited 12.04.2011 by Raphael Dumusc                                        *
 * Incorporated ROS code for improved Bayer Pattern to RGB conversion.        *
 *****************************************************************************/


//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "Bayer.h"
#include <math.h>

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
XnUInt8 Gamma[256] = {
	0,      1,     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
	16,    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
	32,    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
	48,    49,    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
	64,    65,    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
	80,    81,    82,    83,    84,    85,    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
	96,    97,    98,    99,    100,   101,   102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
	112,   113,   114,   115,   116,   117,   118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
	128,   129,   130,   131,   132,   133,   134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
	144,   145,   146,   147,   148,   149,   150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
	160,   161,   162,   163,   164,   165,   166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
	176,   177,   178,   179,   180,   181,   182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
	192,   193,   194,   195,   196,   197,   198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
	208,   209,   210,   211,   212,   213,   214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
	224,   225,   226,   227,   228,   229,   230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
	240,   241,   242,   243,   244,   245,   246,   247,   248,   249,   250,   251,   252,   253,   254,   255};

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
void BayerUpdateGamma(float fGammaCorr)
{
	for(XnUInt32 iG = 0; iG < 256;++iG)
		Gamma[iG] = XnUInt32(255*pow(XnDouble(iG)/255.0,(XnDouble)fGammaCorr) + 0.5);
}


/*
static inline void WriteRGB(XnUInt8 *pBuffer, XnUInt8 nRed, XnUInt8 nGreen, XnUInt8 nBlue)
{
	pBuffer[BAYER_RED] = Gamma[nRed];
	pBuffer[BAYER_GREEN] = Gamma[nGreen];
	pBuffer[BAYER_BLUE] = Gamma[nBlue];
}


void Bayer2RGB888(const XnUInt8* pBayerImage, XnUInt8* pRGBImage, XnUInt32 nXRes, XnUInt32 nYRes, XnUInt32 nDownSampleStep, XnUInt32 nBadPixels)
{
	XnUInt8 nRed;
	XnUInt8 nGreen;
	XnUInt8 nBlue;
	const XnUInt8* pBayer;
	XnUInt8* pRGB;

	//if (nBadPixels > 1)
	//{
		//nBadPixels = 1;
	//}

	XnInt32 BAYER_LINE_LENGTH = nXRes;
	XnInt32 BAYER_LINE_LENGTH2 = BAYER_LINE_LENGTH*2;
	XnInt32 BAYER_RGB_LINE_LENGTH = nXRes*BAYER_BPP;
	XnInt32 BAYER_RGB_LINE_LENGTH2 = BAYER_RGB_LINE_LENGTH*2;

	const XnUInt8* pCurrBayer;
	XnUInt8* pCurrRGB;
	XnUInt32 nColCount;
	XnUInt32 nTotalColsCount = (nXRes-2) / 2;
	XnUInt32 nTotalRowsCount = (nYRes-4) / 2;

	pBayer = pBayerImage + BAYER_LINE_LENGTH - nBadPixels;
	pRGB = pRGBImage + BAYER_RGB_LINE_LENGTH;

	do {
		pCurrBayer = pBayer+ 1;
		pCurrRGB = pRGB + BAYER_BPP;

		nColCount = nTotalColsCount;

		do {

			nRed   = ((XnUInt32)pCurrBayer[-BAYER_LINE_LENGTH]+pCurrBayer[BAYER_LINE_LENGTH]) / 2;
			nBlue  = ((XnUInt32)pCurrBayer[-1]+pCurrBayer[1]) / 2;
			WriteRGB(pCurrRGB+0, nRed, pCurrBayer[0], nBlue);

			nRed   = ((XnUInt32)pCurrBayer[-BAYER_LINE_LENGTH+2]+pCurrBayer[BAYER_LINE_LENGTH+2]) / 2;
			nGreen = ((XnUInt32)pCurrBayer[0]+pCurrBayer[2]) / 2;
			WriteRGB(pCurrRGB+BAYER_BPP, nRed, nGreen, pCurrBayer[1]);

			nGreen = ((XnUInt32)pCurrBayer[BAYER_LINE_LENGTH-1]+pCurrBayer[BAYER_LINE_LENGTH+1]) / 2;
			nBlue  = ((XnUInt32)pCurrBayer[BAYER_LINE_LENGTH2-1]+pCurrBayer[BAYER_LINE_LENGTH2+1]) / 2;
			WriteRGB(pCurrRGB+BAYER_RGB_LINE_LENGTH, pCurrBayer[BAYER_LINE_LENGTH], nGreen, nBlue);

			nRed   = ((XnUInt32)pCurrBayer[BAYER_LINE_LENGTH]+pCurrBayer[BAYER_LINE_LENGTH+2]) / 2;
			nBlue  = ((XnUInt32)pCurrBayer[1]+pCurrBayer[BAYER_LINE_LENGTH2+1]) / 2;
			WriteRGB(pCurrRGB+BAYER_RGB_LINE_LENGTH+BAYER_BPP, nRed, pCurrBayer[BAYER_LINE_LENGTH+1], nBlue);

			pCurrBayer += 2;
			pCurrRGB += 2*BAYER_BPP;
		} while (--nColCount);

		pBayer += BAYER_LINE_LENGTH2;
		pRGB += BAYER_RGB_LINE_LENGTH2;
	} while (--nTotalRowsCount);
}
*/


/*
 The ROS bayer pattern to RGB conversion
 Modified to be used in Avin's mod of the Primesense driver.
 Original code available here:
 http://www.ros.org/doc/api/openni_camera/html/openni__image__bayer__grbg_8cpp_source.html
*/

/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011 2011 Willow Garage, Inc.
 *    Suat Gedikli <gedikli@willowgarage.com>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sstream>
#include <iostream>

#define AVG(a,b) (((int)(a) + (int)(b)) >> 1)
#define AVG3(a,b,c) (((int)(a) + (int)(b) + (int)(c)) / 3)
#define AVG4(a,b,c,d) (((int)(a) + (int)(b) + (int)(c) + (int)(d)) >> 2)
#define WAVG4(a,b,c,d,x,y)  ( ( ((int)(a) + (int)(b)) * (int)(x) + ((int)(c) + (int)(d)) * (int)(y) ) / ( 2 * ((int)(x) + (int(y))) ) )
using namespace std;

typedef enum
{
	Bilinear = 0,
	EdgeAware,
	EdgeAwareWeighted
} DebayeringMethod;


void fillRGB(unsigned width, unsigned height, const XnUInt8* bayer_pixel, unsigned char* rgb_buffer, DebayeringMethod debayering_method, XnUInt32 nDownSampleStep)
{
		
	unsigned rgb_line_step = width * 3;
	
	// padding skip for destination image
	unsigned rgb_line_skip = rgb_line_step - width * 3;
	
	if (nDownSampleStep == 1)
	{
		//register const XnUInt8 *bayer_pixel = image_md_->Data ();
		register unsigned yIdx, xIdx;
		
		int bayer_line_step = width;
		int bayer_line_step2 = width << 1;
		
		if (debayering_method == Bilinear)
		{
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			
			// main processing
			
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}
				
				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r G r g
				// line_step    g b g b
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g R g
				// line_step    g b g b
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);
				
				// BGBG line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g B g b
				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		else if (debayering_method == EdgeAware)
		{
			int dh, dv;
			
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			// main processing
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					
					dh = abs (bayer_pixel[0] - bayer_pixel[2]);
					dv = abs (bayer_pixel[-bayer_line_step + 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dh > dv)
						rgb_buffer[4] = AVG (bayer_pixel[-bayer_line_step + 1], bayer_pixel[bayer_line_step + 1]);
					else if (dv > dh)
						rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[2]);
					else
						rgb_buffer[4] = AVG4 (bayer_pixel[-bayer_line_step + 1], bayer_pixel[bayer_line_step + 1], bayer_pixel[0], bayer_pixel[2]);
					
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					dv = abs (bayer_pixel[0] - bayer_pixel[bayer_line_step2]);
					dh = abs (bayer_pixel[bayer_line_step - 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv > dh)
						rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					else if (dh > dv)
						rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step2]);
					else
						rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}
				
				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r G r g
				// line_step    g b g b
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g R g
				// line_step    g b g b
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);
				
				// BGBG line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g B g b
				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		else if (debayering_method == EdgeAwareWeighted)
		{
			int dh, dv;
			
			// first two pixel values for first two lines
			// Bayer         0 1 2
			//         0     G r g
			// line_step     b g b
			// line_step2    g r g
			
			rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//         0     g R g
			// line_step     b g b
			// line_step2    g r g
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			// BGBG line
			// Bayer         0 1 2
			//         0     g r g
			// line_step     B g b
			// line_step2    g r g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// pixel (1, 1)  0 1 2
			//         0     g r g
			// line_step     b G b
			// line_step2    g r g
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the first two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer        -1 0 1 2
				//           0   r G r g
				//   line_step   g b g b
				// line_step2    r g r g
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = bayer_pixel[bayer_line_step + 1];
				
				// Bayer        -1 0 1 2
				//          0    r g R g
				//  line_step    g b g b
				// line_step2    r g r g
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				// BGBG line
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g B g b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// Bayer         -1 0 1 2
				//         0      r g r g
				// line_step      g b G b
				// line_step2     r g r g
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = AVG( bayer_pixel[line_step] , bayer_pixel[line_step+2] );
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer        -1 0 1
			//           0   r G r
			//   line_step   g b g
			// line_step2    r g r
			rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
			
			// Bayer        -1 0 1
			//          0    r g R
			//  line_step    g b g
			// line_step2    r g r
			rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[5] = bayer_pixel[line_step];
			
			// BGBG line
			// Bayer        -1 0 1
			//          0    r g r
			//  line_step    g B g
			// line_step2    r g r
			rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
			rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
			
			// Bayer         -1 0 1
			//         0      r g r
			// line_step      g b G
			// line_step2     r g r
			rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
			
			bayer_pixel += bayer_line_step + 2;
			rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			// main processing
			for (yIdx = 2; yIdx < height - 2; yIdx += 2)
			{
				// first two pixel values
				// Bayer         0 1 2
				//        -1     b g b
				//         0     G r g
				// line_step     b g b
				// line_step2    g r g
				
				rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
				rgb_buffer[1] = bayer_pixel[0]; // green pixel
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]); // blue;
				
				// Bayer         0 1 2
				//        -1     b g b
				//         0     g R g
				// line_step     b g b
				// line_step2    g r g
				//rgb_pixel[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
				
				// BGBG line
				// Bayer         0 1 2
				//         0     g r g
				// line_step     B g b
				// line_step2    g r g
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[bayer_line_step2]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				// pixel (1, 1)  0 1 2
				//         0     g r g
				// line_step     b G b
				// line_step2    g r g
				//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				
				rgb_buffer += 6;
				bayer_pixel += 2;
				// continue with rest of the line
				for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
				{
					// GRGR line
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//           0   r G r g
					//   line_step   g b g b
					// line_step2    r g r g
					rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
					rgb_buffer[1] = bayer_pixel[0];
					rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
					
					// Bayer        -1 0 1 2
					//          -1   g b g b
					//          0    r g R g
					//  line_step    g b g b
					// line_step2    r g r g
					
					dh = abs (bayer_pixel[0] - bayer_pixel[2]);
					dv = abs (bayer_pixel[-bayer_line_step + 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv == 0 && dh == 0)
						rgb_buffer[4] = AVG4 (bayer_pixel[1 - bayer_line_step], bayer_pixel[1 + bayer_line_step], bayer_pixel[0], bayer_pixel[2]);
					else
						rgb_buffer[4] = WAVG4 (bayer_pixel[1 - bayer_line_step], bayer_pixel[1 + bayer_line_step], bayer_pixel[0], bayer_pixel[2], dh, dv);
					rgb_buffer[3] = bayer_pixel[1];
					rgb_buffer[5] = AVG4 (bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step], bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
					
					// BGBG line
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g B g b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
					rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
					
					dv = abs (bayer_pixel[0] - bayer_pixel[bayer_line_step2]);
					dh = abs (bayer_pixel[bayer_line_step - 1] - bayer_pixel[bayer_line_step + 1]);
					
					if (dv == 0 && dh == 0)
						rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
					else
						rgb_buffer[rgb_line_step + 1] = WAVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1], dh, dv);
					
					// Bayer         -1 0 1 2
					//         -1     g b g b
					//          0     r g r g
					// line_step      g b G b
					// line_step2     r g r g
					rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
					rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
					rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
				}
				
				// last two pixels of the line
				// last two pixel values for first two lines
				// GRGR line
				// Bayer        -1 0 1
				//           0   r G r
				//   line_step   g b g
				// line_step2    r g r
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = rgb_buffer[5] = rgb_buffer[2] = bayer_pixel[bayer_line_step];
				
				// Bayer        -1 0 1
				//          0    r g R
				//  line_step    g b g
				// line_step2    r g r
				rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[5] = bayer_pixel[line_step];
				
				// BGBG line
				// Bayer        -1 0 1
				//          0    r g r
				//  line_step    g B g
				// line_step2    r g r
				rgb_buffer[rgb_line_step ] = AVG4 (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1], bayer_pixel[-1], bayer_pixel[bayer_line_step2 - 1]);
				rgb_buffer[rgb_line_step + 1] = AVG4 (bayer_pixel[0], bayer_pixel[bayer_line_step2], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				//rgb_pixel[rgb_line_step + 2] = bayer_pixel[line_step];
				
				// Bayer         -1 0 1
				//         0      r g r
				// line_step      g b G
				// line_step2     r g r
				rgb_buffer[rgb_line_step + 3] = AVG (bayer_pixel[1], bayer_pixel[bayer_line_step2 + 1]);
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
				
				bayer_pixel += bayer_line_step + 2;
				rgb_buffer += rgb_line_step + 6 + rgb_line_skip;
			}
			
			//last two lines
			// Bayer         0 1 2
			//        -1     b g b
			//         0     G r g
			// line_step     b g b
			
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[rgb_line_step ] = rgb_buffer[3] = rgb_buffer[0] = bayer_pixel[1]; // red pixel
			rgb_buffer[1] = bayer_pixel[0]; // green pixel
			rgb_buffer[rgb_line_step + 2] = rgb_buffer[2] = bayer_pixel[bayer_line_step]; // blue;
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g R g
			// line_step     b g b
			//rgb_pixel[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
			rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[2 - bayer_line_step]);
			
			// BGBG line
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     B g b
			//rgb_pixel[rgb_line_step    ] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 1] = AVG (bayer_pixel[0], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer         0 1 2
			//        -1     b g b
			//         0     g r g
			// line_step     b G b
			//rgb_pixel[rgb_line_step + 3] = AVG( bayer_pixel[1] , bayer_pixel[line_step2+1] );
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			
			rgb_buffer += 6;
			bayer_pixel += 2;
			// rest of the last two lines
			for (xIdx = 2; xIdx < width - 2; xIdx += 2, rgb_buffer += 6, bayer_pixel += 2)
			{
				// GRGR line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r G r g
				// line_step    g b g b
				rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
				rgb_buffer[1] = bayer_pixel[0];
				rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g R g
				// line_step    g b g b
				rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
				rgb_buffer[4] = AVG4 (bayer_pixel[0], bayer_pixel[2], bayer_pixel[bayer_line_step + 1], bayer_pixel[1 - bayer_line_step]);
				rgb_buffer[5] = AVG4 (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2], bayer_pixel[-bayer_line_step], bayer_pixel[-bayer_line_step + 2]);
				
				// BGBG line
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g B g b
				rgb_buffer[rgb_line_step ] = AVG (bayer_pixel[-1], bayer_pixel[1]);
				rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
				rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
				
				
				// Bayer       -1 0 1 2
				//        -1    g b g b
				//         0    r g r g
				// line_step    g b G b
				//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
				rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
				rgb_buffer[rgb_line_step + 5] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[bayer_line_step + 2]);
			}
			
			// last two pixel values for first two lines
			// GRGR line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r G r
			// line_step    g b g
			rgb_buffer[rgb_line_step ] = rgb_buffer[0] = AVG (bayer_pixel[1], bayer_pixel[-1]);
			rgb_buffer[1] = bayer_pixel[0];
			rgb_buffer[5] = rgb_buffer[2] = AVG (bayer_pixel[bayer_line_step], bayer_pixel[-bayer_line_step]);
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g R
			// line_step    g b g
			rgb_buffer[rgb_line_step + 3] = rgb_buffer[3] = bayer_pixel[1];
			rgb_buffer[4] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step + 1], bayer_pixel[-bayer_line_step + 1]);
			//rgb_pixel[5] = AVG( bayer_pixel[line_step], bayer_pixel[-line_step] );
			
			// BGBG line
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g B g
			//rgb_pixel[rgb_line_step    ] = AVG2( bayer_pixel[-1], bayer_pixel[1] );
			rgb_buffer[rgb_line_step + 1] = AVG3 (bayer_pixel[0], bayer_pixel[bayer_line_step - 1], bayer_pixel[bayer_line_step + 1]);
			rgb_buffer[rgb_line_step + 5] = rgb_buffer[rgb_line_step + 2] = bayer_pixel[bayer_line_step];
			
			// Bayer       -1 0 1
			//        -1    g b g
			//         0    r g r
			// line_step    g b G
			//rgb_pixel[rgb_line_step + 3] = bayer_pixel[1];
			rgb_buffer[rgb_line_step + 4] = bayer_pixel[bayer_line_step + 1];
			//rgb_pixel[rgb_line_step + 5] = bayer_pixel[line_step];
		}
		//else
		//	THROW_OPENNI_EXCEPTION ("Unknwon debayering method: %d", (int)debayering_method);
	}
	//Warning: Downsampling mod is untested
	else if (nDownSampleStep > 1)
	{		
		// get each or each 2nd pixel group to find rgb values!
		register unsigned bayerXStep = nDownSampleStep;
		register unsigned bayerYSkip = (nDownSampleStep - 1) * width;
		
		// Downsampling and debayering at once
		register const XnUInt8* bayer_buffer = bayer_pixel;
		
		for (register unsigned yIdx = 0; yIdx < height; ++yIdx, bayer_buffer += bayerYSkip, rgb_buffer += rgb_line_skip) // skip a line
		{
			for (register unsigned xIdx = 0; xIdx < width; ++xIdx, rgb_buffer += 3, bayer_buffer += bayerXStep)
			{
				rgb_buffer[ 2 ] = bayer_buffer[ width ];
				rgb_buffer[ 1 ] = AVG (bayer_buffer[0], bayer_buffer[ width + 1]);
				rgb_buffer[ 0 ] = bayer_buffer[ 1 ];
			}
		}
	}
}



void Bayer2RGB888(const XnUInt8* pBayerImage, XnUInt8* pRGBImage, XnUInt32 nXRes, XnUInt32 nYRes, XnUInt32 nDownSampleStep, XnUInt32 nBadPixels)
{	
	fillRGB(nXRes, nYRes, pBayerImage, pRGBImage, DebayeringMethod(2), nDownSampleStep); // DebayeringMethod(0) == bilinear, (1) == edge aware, (2) == edge aware weighted
}



