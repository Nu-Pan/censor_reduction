
#include "stdafx.h"

#include "opencv_utils.hpp"

#include "macro_utils.hpp"
#include "opencv_utils.hpp"
#include "mat_image_iterator.hpp"

using namespace std;
using namespace cv;

//-------------------------------- CGuiTrackbarInteger --------------------------------

/** コンストラクタ
*/
censor_reduction::CGuiTrackbarInteger::CGuiTrackbarInteger(const std::string& trackbar_name, const std::string& window_name, int max)
:_iValue(0)
,_iMax(max)
{
	cv::createTrackbar(trackbar_name.c_str(), window_name.c_str(), &_iValue, _iMax);
}

//-------------------------------- CGuiTrackbarFloat --------------------------------

const int censor_reduction::CGuiTrackbarFloat::_iMax = 1000000;

/** コンストラクタ
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
	//トリムサイズを決定
	const int OverWidth = source.size().width%dominator;
	const int OverHeight = source.size().height%dominator;
	const int TrimWidth = source.size().width - OverWidth;
	const int TrimHeight = source.size().height - OverHeight;
	const cv::Size TrimSize(TrimWidth, TrimHeight);

	UTL_PRMCHK(0 < TrimWidth);
	UTL_PRMCHK(0 < TrimHeight);

	//トリム不要ならそのまま画像を返す
	if (TrimSize == source.size())
	{
		return source;
	}

	//ROIでトリムしてクローン
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

	//メイン処理
	Mat Censored(source.size(), source.type());
	vector<uint32_t> Buffer32(NoC);
	vector<uint8_t> Buffer8;
	Buffer8.reserve(NoC);
	Rect Roi;
	Roi.width = block_size;
	Roi.height = block_size;
	//すべてのブロックについてループ
	for (Roi.y = 0; Roi.y < Res.height; Roi.y += block_size) {
		for (Roi.x = 0; Roi.x < Res.width; Roi.x += block_size) {
			//ブロック内の輝度値の平均を求める
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
			//書き込み先ブロックの1行目をmemcpyで平均色塗りつぶし
			for (CMatImageIterator<uint8_t> i(Censored, Rect(Roi.x, Roi.y, block_size, 1)); i; ++i)
			{
				memcpy(i.pPixel(), Buffer8.data(), PixelByteSize);
			}
			//書き込み先ブロックの2行目以降をmemcpyで平均色塗りつぶし
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

	//縦解像度1のシマシマ帯を2つ作る(互いに反転パターン)
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

	//帯をコピーしてチェスボードパターンを生成
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
	//すべての画像のフォーマットが同一か, 不正なフォーマットでないか, のチェック
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

	//作業用にコピーを取る
	BlendImagesType Images = images;

	//ブレンド率を正規化
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

	//ブレンディングを行う
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

	//ブレンディング結果をBGR24に飽和演算でパック
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
