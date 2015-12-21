
#pragma once

#include "macro_utils.hpp"

namespace censor_reduction {
	typedef std::vector<std::pair<float, cv::Mat> > BlendImagesType;

	/** �����l������GUI�g���b�N�o�[���b�p�[
	*/
	class CGuiTrackbarInteger
	{
	private:
		int _iValue;
		int _iMax;
		//TODO �ŏ��l�̐ݒ�

	public:
		/** �R���X�g���N�^
		*/
		CGuiTrackbarInteger(const std::string& trackbar_name, const std::string& window_name, int max);

		/** �g���b�N�o�[�̎����l�𓾂�
		*/
		int GetValue() const
		{
			return _iValue;
		}
	};

	/** �����l������GUI�g���b�N�o�[���b�p�[
	*/
	class CGuiTrackbarFloat
	{
	private:
		static const int _iMax;
		int _iValue;
		float _fMin;
		float _fMax;
		//TODO �ŏ��l�̐ݒ�

	public:
		/** �R���X�g���N�^
		*/
		CGuiTrackbarFloat(const std::string& trackbar_name, const std::string& window_name, float min, float max);

		/** �g���b�N�o�[�̎����l�𓾂�
		*/
		float GetValue() const
		{
			return _fMin+static_cast<float>(_iValue)*(_fMax-_fMin)/static_cast<int>(_iMax);
		}
	};

	/** �摜�̋P�x�l(�v�f)�P�̃o�C�g���𓾂�
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

	/** dominator�Ŋ���؂��摜�T�C�Y�ł����true
	*/
	inline bool IsDivisible(const cv::Size& resolution, int dominator)
	{
		UTL_PRMCHK( 0<dominator );
		return resolution.width%dominator == 0 && resolution.height%dominator == 0;
	}

	/** dominator�Ŋ���؂��摜�T�C�Y�ł����true
	*/
	inline bool IsDivisible(const cv::Mat& source, int dominator)
	{
		return IsDivisible(source.size(), dominator);
	}

	/** �w��摜�̎w����W�s�N�Z���̃|�C���^�𓾂�
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

	/** dominator�Ŋ���؂��T�C�Y�ɂȂ�悤�ɉ摜���g���~���O����
	* @param [in] source	�����Ώۉ摜
	* @param [in] dominator	����
	* @return				�g���~���O�ς݉摜
	*/
	cv::Mat CreateTrimmedDivisibleImage(const cv::Mat& source, int dominator);

	/** �S�̂Ƀ��U�C�N�̂��������摜�𐶐�����
	* @param [in] source		�����Ώۉ摜
	* @param [in] block_size	���U�C�N��1�u���b�N�̃T�C�Y
	* @return					���U�C�N�̂��������摜
	*/
	cv::Mat CreateCensoredImage(const cv::Mat& source, int block_size);

	/** �w��u���b�N�T�C�Y�̃`�F�X�{�[�h�p�^�[���𐶐�����
	*/
	cv::Mat CreateChessboradPatternImage(const cv::Size& resolution, int block_size);

	/** �w��u�����h���ŕ����̉摜���u�����h����
	* ���ׂẲ摜�T�C�Y�������Ă���K�v������
	* ���ׂẴu�����h���̘a��1�ɂȂ�悤�ɐ��K�������
	* �����v�Z��float�ōs�����ߒᑬ
	*/
	cv::Mat BlendImages(const BlendImagesType& images);
};
