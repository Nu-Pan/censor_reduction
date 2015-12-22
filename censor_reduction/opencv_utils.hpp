
#pragma once

#include "macro_utils.hpp"

namespace censor_reduction {
	typedef std::vector<std::pair<float, cv::Mat> > BlendImagesType;

	/** 整数値を扱うGUIトラックバーラッパー
	*/
	class CGuiTrackbarInteger
	{
	private:
		int _iValue;
		int _iMax;
		//TODO 最小値の設定

	public:
		/** コンストラクタ
		*/
		CGuiTrackbarInteger(const std::string& trackbar_name, const std::string& window_name, int max);

		/** トラックバーの示す値を得る
		*/
		int GetValue() const
		{
			return _iValue;
		}
	};

	/** 整数値を扱うGUIトラックバーラッパー
	*/
	class CGuiTrackbarFloat
	{
	private:
		static const int _iMax;
		int _iValue;
		float _fMin;
		float _fMax;
		//TODO 最小値の設定

	public:
		/** コンストラクタ
		*/
		CGuiTrackbarFloat(const std::string& trackbar_name, const std::string& window_name, float min, float max);

		/** トラックバーの示す値を得る
		*/
		float GetValue() const
		{
			return _fMin+static_cast<float>(_iValue)*(_fMax-_fMin)/static_cast<int>(_iMax);
		}
	};

	/** 画像の輝度値(要素)１つのバイト数を得る
	*/
	inline int GetImageElementByteSize(const cv::Mat& source)
	{
		using namespace std;

		switch (source.depth())
		{
		case CV_8S:
		case CV_8U:
			return 1;
			break;
		case CV_16S:
		case CV_16U:
			return 2;
			break;
		case CV_32S:
			//case CV_32U:
		case CV_32F:
			return 4;
			break;
		case CV_64F:
			return 8;
			break;
		default:
			ostringstream oss;
			oss << "Unknown image depth." << endl;
			oss << "source.depth() = " << source.depth() << endl;
			THROW_OSS(oss);
			break;
		}
		return 0;
	}

	/** dominatorで割り切れる画像サイズであればtrue
	*/
	inline bool IsDivisible(const cv::Size& resolution, int dominator)
	{
		UTL_PRMCHK( 0<dominator );
		return resolution.width%dominator == 0 && resolution.height%dominator == 0;
	}

	/** dominatorで割り切れる画像サイズであればtrue
	*/
	inline bool IsDivisible(const cv::Mat& source, int dominator)
	{
		return IsDivisible(source.size(), dominator);
	}

	/** 指定画像の指定座標ピクセルのポインタを得る
	*/
	template<typename T>
	T* PixelAt(const cv::Mat& image, int u, int v)
	{
		UTL_ASSERT(GetImageElementByteSize(image) == sizeof(T));
		UTL_ASSERT(0 <= u);
		UTL_ASSERT(0 <= v);
		UTL_ASSERT(u < image.size().width());
		UTL_ASSERT(v < image.size().height());
		return reinterpret_cast<T*>(image.data + v*image.step + u*image.channels()*sizeof(T));
	}

	/** dominatorで割り切れるサイズになるように画像をトリミングする
	* @param [in] source	処理対象画像
	* @param [in] dominator	分母
	* @return				トリミング済み画像
	*/
	cv::Mat CreateTrimmedDivisibleImage(const cv::Mat& source, int dominator);

	/** 全体にモザイクのかかった画像を生成する
	* @param [in] source		処理対象画像
	* @param [in] block_size	モザイクの1ブロックのサイズ
	* @return					モザイクのかかった画像
	*/
	cv::Mat CreateCensoredImage(const cv::Mat& source, int block_size);

	/** 指定ブロックサイズのチェスボードパターンを生成する
	*/
	cv::Mat CreateChessboradPatternImage(const cv::Size& resolution, int block_size);

	/** 指定ブレンド率で複数の画像をブレンドする
	* すべての画像サイズが揃っている必要がある
	* すべてのブレンド率の和が1になるように正規化される
	* 内部計算をfloatで行うため低速
	*/
	cv::Mat BlendImages(const BlendImagesType& images);
};
