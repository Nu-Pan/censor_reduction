
#include "stdafx.h"

#include "opencv_utils.hpp"

#include "macro_utils.hpp"
#include "opencv_utils.hpp"
#include "mat_image_iterator.hpp"

using namespace std;
using namespace cv;

//-------------------------------- CGuiTrackbarInteger --------------------------------

/** �R���X�g���N�^
*/
censor_reduction::CGuiTrackbarInteger::CGuiTrackbarInteger(const std::string& trackbar_name, const std::string& window_name, int max)
:_iValue(0)
,_iMax(max)
{
	cv::createTrackbar(trackbar_name.c_str(), window_name.c_str(), &_iValue, _iMax);
}

//-------------------------------- CGuiTrackbarFloat --------------------------------

const int censor_reduction::CGuiTrackbarFloat::_iMax = 1000000;

/** �R���X�g���N�^
*/
censor_reduction::CGuiTrackbarFloat::CGuiTrackbarFloat(const std::string& trackbar_name, const std::string& window_name, float min, float max)
:_iValue(0)
,_fMin(min)
,_fMax(max)
{
	cv::createTrackbar(trackbar_name.c_str(), window_name.c_str(), &_iValue, _iMax);
}

//-------------------------------- public functions --------------------------------

cv::Mat censor_reduction::CreateTrimmedDivisibleImage(const cv::Mat& source, int dominator)
{
	//�g�����T�C�Y������
	const int OverWidth = source.size().width%dominator;
	const int OverHeight = source.size().height%dominator;
	const int TrimWidth = source.size().width - OverWidth;
	const int TrimHeight = source.size().height - OverHeight;
	const cv::Size TrimSize(TrimWidth, TrimHeight);

	UTL_PRMCHK(0 < TrimWidth);
	UTL_PRMCHK(0 < TrimHeight);

	//�g�����s�v�Ȃ炻�̂܂܉摜��Ԃ�
	if (TrimSize == source.size())
	{
		return source;
	}

	//ROI�Ńg�������ăN���[��
	return cv::Mat(source, cv::Rect(OverWidth / 2, OverHeight / 2, TrimWidth, TrimHeight)).clone();
}

cv::Mat censor_reduction::CreateCensoredImage(const cv::Mat& source, int block_size)
{
	UTL_PRMCHK(IsDivisible(source, block_size));
	UTL_PRMCHK(GetImageElementByteSize(source) == sizeof(uint8_t));

	const Size Res = source.size();
	const int NoC = source.channels();
	const int BlockArea = block_size*block_size;
	const int PixelByteSize = NoC*sizeof(uint8_t);
	const int LineByteSizeInBlock = PixelByteSize*block_size;

	//���C������
	Mat Censored(source.size(), source.type());
	vector<uint32_t> Buffer32(NoC);
	vector<uint8_t> Buffer8;
	Buffer8.reserve(NoC);
	Rect Roi;
	Roi.width = block_size;
	Roi.height = block_size;
	//���ׂẴu���b�N�ɂ��ă��[�v
	for (Roi.y = 0; Roi.y < Res.height; Roi.y += block_size) {
		for (Roi.x = 0; Roi.x < Res.width; Roi.x += block_size) {
			//�u���b�N���̋P�x�l�̕��ς����߂�
			for (vector<unsigned>::iterator i = Buffer32.begin(); i != Buffer32.end(); ++i)
			{
				*i = 0;
			}
			for (CMatImageIterator<uint8_t> i(source, Roi); i; ++i)
			{
				for (int c = 0; c < NoC; ++c)
				{
					Buffer32[c] += i[c];
				}
			}
			Buffer8.clear();
			for (vector<unsigned>::iterator i = Buffer32.begin(); i != Buffer32.end(); ++i)
			{
				Buffer8.push_back(*i / BlockArea);
			}
			//�������ݐ�u���b�N��1�s�ڂ�memcpy�ŕ��ϐF�h��Ԃ�
			for (CMatImageIterator<uint8_t> i(Censored, Rect(Roi.x, Roi.y, block_size, 1)); i; ++i)
			{
				memcpy(i.pPixel(), Buffer8.data(), PixelByteSize);
			}
			//�������ݐ�u���b�N��2�s�ڈȍ~��memcpy�ŕ��ϐF�h��Ԃ�
			for (CMatImageIterator<uint8_t> i(Censored, Rect(Roi.x, Roi.y + 1, 1, block_size - 1)); i; ++i)
			{
				memcpy(i.pPixel(), i.pPixelOffsetV(-1), LineByteSizeInBlock);
			}
		}
	}

	return Censored;
}

cv::Mat censor_reduction::CreateChessboradPatternImage(const cv::Size& resolution, int block_size)
{
	UTL_PRMCHK(0 < resolution.width);
	UTL_PRMCHK(0 < resolution.height);
	UTL_PRMCHK(0 < block_size);
	UTL_PRMCHK(IsDivisible(resolution, block_size));

	const int Width = resolution.width;
	const int Height = resolution.height;
	const int NoC = 3;
	const int NoEInLine = NoC * Width;
	const int NoEInBlock = NoC * block_size;
	const int NoBInLine = Width / block_size;
	const int PixelByteSize = NoC*sizeof(uint8_t);
	const int LineByteSize = PixelByteSize * Width;
	const int LineByteSizeInBlock = PixelByteSize*block_size;

	uint8_t Scaler;

	//�c�𑜓x1�̃V�}�V�}�т�2���(�݂��ɔ��]�p�^�[��)
	vector<uint8_t> SlitPN(NoEInLine);
	Scaler = 0x00;
	for (vector<uint8_t>::iterator i=SlitPN.begin(); i != SlitPN.end(); i += NoEInBlock) {
		Scaler = ~Scaler;
		memset(i._Ptr, Scaler, LineByteSizeInBlock);
	}
	vector<uint8_t> SlitNP(NoEInLine);
	Scaler = 0xFF;
	for (vector<uint8_t>::iterator i = SlitNP.begin(); i != SlitNP.end(); i += NoEInBlock) {
		Scaler = ~Scaler;
		memset(i._Ptr, Scaler, LineByteSizeInBlock);
	}

	//�т��R�s�[���ă`�F�X�{�[�h�p�^�[���𐶐�
	cv::Mat Left(resolution, CV_8UC3);
	for (CMatImageIterator<uint8_t> i(Left, Rect(0, 0, 1, Height)); i; ++i)
	{
		if ((i.v() / block_size) & 1)
		{
			memcpy(i.pPixel(), SlitPN.data(), LineByteSize);
		}
		else
		{
			memcpy(i.pPixel(), SlitNP.data(), LineByteSize);
		}
	}

	return Left;
}

cv::Mat censor_reduction::BlendImages(const BlendImagesType& images)
{
	//���ׂẲ摜�̃t�H�[�}�b�g�����ꂩ, �s���ȃt�H�[�}�b�g�łȂ���, �̃`�F�b�N
	const Size ImageSize = images.front().second.size();
	const int ImageChannels = images.front().second.channels();
	{
		for (BlendImagesType::const_iterator i = images.begin()+1; i != images.end(); ++i)
		{
			UTL_PRMCHK(ImageSize == i->second.size());
			UTL_PRMCHK(ImageChannels == i->second.channels());
			UTL_PRMCHK(GetImageElementByteSize(i->second) == sizeof(uint8_t));
		}
	}

	//��Ɨp�ɃR�s�[�����
	BlendImagesType Images = images;

	//�u�����h���𐳋K��
	{
		float SumOfRate = 0.f;
		for (BlendImagesType::iterator i = Images.begin(); i != Images.end(); ++i)
		{
			SumOfRate += i->first;
		}
		for (BlendImagesType::iterator i = Images.begin(); i != Images.end(); ++i)
		{
			i->first /= SumOfRate;
		}
	}

	//�u�����f�B���O���s��
	Mat Buffer = Mat::zeros(ImageSize, CV_32FC3);
	for (BlendImagesType::iterator i = Images.begin(); i != Images.end(); ++i)
	{
		const float BlendRate = i->first;
		const Mat SrcImg = i->second;
		CMatImageIterator<uint8_t> SrcIter(SrcImg);
		CMatImageIterator<float> DstIter(Buffer);
		for (;;)
		{
			if (!SrcIter || !DstIter)
			{
				break;
			}
			for (int c = 0; c < ImageChannels; ++c)
			{
				DstIter[c] += BlendRate * static_cast<float>(SrcIter[c]);
			}
			++SrcIter;
			++DstIter;
		}
	}

	//�u�����f�B���O���ʂ�BGR24�ɖO�a���Z�Ńp�b�N
	Mat Left(ImageSize, CV_8UC3);
	CMatImageIterator<float> SrcIter(Buffer);
	CMatImageIterator<uint8_t> DstIter(Left);
	for (;;)
	{
		if (!SrcIter || !DstIter)
		{
			break;
		}
		for (int c = 0; c < ImageChannels; ++c)
		{
			if (255.0 < SrcIter[c])
			{
				DstIter[c] = 255;
			}
			else if (SrcIter[c] < 0)
			{
				DstIter[c] = 0;
			}
			else
			{
				DstIter[c] = static_cast<uint8_t>(SrcIter[c]);
			}
		}
		++SrcIter;
		++DstIter;
	}

	return Left;
}
