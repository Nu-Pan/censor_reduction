
#pragma once

#include "opencv_utils.hpp"

namespace censor_reduction{
	/** cv::Mat�摜�C�e���[�^
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
		/** �R���X�g���N�^
		* �������q���X�g���̈ˑ��֌W�ɒ���
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

		/** �R���X�g���N�^
		* ROI�Ɍ��肵�ăC�e���[�V�������s��
		* �������q���X�g���̈ˑ��֌W�ɒ���
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

		/** �L���ȃs�N�Z�����w���Ă����true��ԋp
		* �摜�I�[�ɒB���Ă����false��ԋp
		*/
		operator bool() const
		{
			return _v != _ImageResolution.height;
		}

		/** ���̃s�N�Z���֐i��
		*/
		CMatImageIterator& operator ++()
		{
			//���łɏI�[�ɒB���Ă���ɂ�������炸��ɐi�����Ƃ��Ă��Ȃ����`�F�b�N
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

		/** ���ݎw���Ă���s�N�Z�����w���|�C���^�𓾂�
		*/
		T* pPixel() const
		{
			return reinterpret_cast<T*>(_pCurrent);
		}

		/** ���ݎw���Ă���s�N�Z����offset�㉺�̃s�N�Z�����w���|�C���^�𓾂�
		*/
		T* pPixelOffsetV(int offset) const
		{
			UTL_ASSERT(0 <= _v + offset);
			UTL_ASSERT(_v + offset < _ImageResolution.height);
			return reinterpret_cast<T*>(_pCurrent + _OriginalHolder.step*offset);
		}

		/** ���ݎw���Ă���s�N�Z����offset���E�̃s�N�Z�����w���|�C���^�𓾂�
		*/
		T* pPixelOffsetU(int offset) const
		{
			UTL_ASSERT(0 <= _u + offset);
			UTL_ASSERT(_u + offset < _ImageResolution.width);
			return reinterpret_cast<T*>(_pCurrent + _PixelByteSize*offset);
		}

		/** �`�����l�������w�肵�Č��ݎw���Ă���s�N�Z���̋P�x�l�̎Q�Ƃ𓾂�
		*/
		T& operator [](int ch) const
		{
			return reinterpret_cast<T*>(_pCurrent)[ch];
		}

		/** ���݂�U���W�𓾂�
		*/
		int u() const
		{
			return _u;
		}

		/** ���݂�V���W�𓾂�
		*/
		int v() const
		{
			return _v;
		}
	};
};