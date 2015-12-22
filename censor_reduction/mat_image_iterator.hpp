
#pragma once

#include "opencv_utils.hpp"

namespace censor_reduction{
	/** cv::Mat画像イテレータ
	*/
	template<typename T>
	class CMatImageIterator
	{
	private:
		cv::Mat _OriginalHolder;
		cv::Size _ImageResolution;
		int _PixelByteSize;			//!< number of channels
		int _GapByteSize;
		uint8_t* _pCurrent;
		int _u;
		int _v;

	public:
		/** コンストラクタ
		* 初期化子リスト内の依存関係に注意
		*/
		explicit CMatImageIterator(const cv::Mat& image)
			:_OriginalHolder(image)
			, _ImageResolution(image.size())
			, _PixelByteSize(image.channels()*GetImageElementByteSize(image))
			, _GapByteSize(image.step - _ImageResolution.width*_PixelByteSize)
			, _pCurrent(image.data)
			, _u(0)
			, _v(0)
		{
			UTL_PRMCHK(GetImageElementByteSize(image) == sizeof(T));
			UTL_PRMCHK(0 < image.size().width);
			UTL_PRMCHK(0 < image.size().height);
			UTL_PRMCHK(0 < image.channels());
		}

		/** コンストラクタ
		* ROIに限定してイテレーションを行う
		* 初期化子リスト内の依存関係に注意
		*/
		CMatImageIterator(const cv::Mat& image, const cv::Rect& roi)
			:_OriginalHolder(image)
			, _ImageResolution(roi.size())
			, _PixelByteSize(image.channels()*GetImageElementByteSize(image))
			, _GapByteSize(image.step - _ImageResolution.width*_PixelByteSize)
			, _pCurrent(image.data + image.step*roi.y + _PixelByteSize*roi.x)
			, _u(0)
			, _v(0)
		{
			UTL_PRMCHK(GetImageElementByteSize(image) == sizeof(T));
			UTL_PRMCHK(0 < image.size().width);
			UTL_PRMCHK(0 < image.size().height);
			UTL_PRMCHK(0 < image.channels());
		}

		/** 有効なピクセルを指していればtrueを返却
		* 画像終端に達していればfalseを返却
		*/
		operator bool() const
		{
			return _v != _ImageResolution.height;
		}

		/** 次のピクセルへ進む
		*/
		CMatImageIterator& operator ++()
		{
			//すでに終端に達しているにもかかわらず先に進もうとしていないかチェック
			UTL_ASSERT( static_cast<bool>(*this) );
			_pCurrent += _PixelByteSize;
			++_u;
			if (_u == _ImageResolution.width)
			{
				_pCurrent += _GapByteSize;
				_u = 0;
				++_v;
			}
			return *this;
		}

		/** 現在指しているピクセルを指すポインタを得る
		*/
		T* pPixel() const
		{
			return reinterpret_cast<T*>(_pCurrent);
		}

		/** 現在指しているピクセルのoffset個上下のピクセルを指すポインタを得る
		*/
		T* pPixelOffsetV(int offset) const
		{
			UTL_ASSERT(0 <= _v + offset);
			UTL_ASSERT(_v + offset < _ImageResolution.height);
			return reinterpret_cast<T*>(_pCurrent + _OriginalHolder.step*offset);
		}

		/** 現在指しているピクセルのoffset個左右のピクセルを指すポインタを得る
		*/
		T* pPixelOffsetU(int offset) const
		{
			UTL_ASSERT(0 <= _u + offset);
			UTL_ASSERT(_u + offset < _ImageResolution.width);
			return reinterpret_cast<T*>(_pCurrent + _PixelByteSize*offset);
		}

		/** チャンネル数を指定して現在指しているピクセルの輝度値の参照を得る
		*/
		T& operator [](int ch) const
		{
			return reinterpret_cast<T*>(_pCurrent)[ch];
		}

		/** 現在のU座標を得る
		*/
		int u() const
		{
			return _u;
		}

		/** 現在のV座標を得る
		*/
		int v() const
		{
			return _v;
		}
	};
};
