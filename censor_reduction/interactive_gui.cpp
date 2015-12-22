
#include "stdafx.hpp"

#include "interactive_gui.hpp"

#include "opencv_utils.hpp"

using namespace std;
using namespace cv;
using namespace censor_reduction;

//-------------------------------- link local variables --------------------------------

namespace
{
	const int _MaxCensorBlockSize = 32;

	mutex _LinkLocalGuard;

	bool _IsTerminate;

	Mat _SourceImage;
	Mat _ResultImage;

	CGuiTrackbarFloat _tbSourceBlendRate;
	CGuiTrackbarFloat _tbCensorBlendRate;
	CGuiTrackbarInteger _tbCensorBlockSize;
	CGuiTrackbarFloat _tbChessboardBlendRate;
	CGuiTrackbarInteger _tbChessboardBlockSize;
	CGuiTrackbarFloat _tbBlurSigma;
};

//-------------------------------- link local functions --------------------------------

namespace
{
	/** 画像処理スレッド関数
	 */
	void ImageProcessingTHreadHandler()
	{
		//XXX 消えるはず : GUIで制御するパラメータ類
		int iCensorBlockSize = 1;
		int iChessboardBlockSize = 1;

		//画像
		Mat Source;
		Mat Result;
		Mat Trimmed;
		Mat Censored;
		Mat ChessBoard;
		Mat Blended;

		//入出力画像には初期値を与える
		{
			lock_guard<mutex> Lock(_LinkLocalGuard);
			Source = _SourceImage;
			Result = _ResultImage;
		}

		//画像処理のメインループ
		for(;;)
		{
			//GUIスレッドとのやりとり
			{
				lock_guard<mutex> Lock(_LinkLocalGuard);
				if( _IsTerminate == true )
				{
					break;
				}
				_ResultImage = Result.clone();
				Source = _SourceImage.clone();
			}

			//XXX トラックバーラッパーでなんとかするべき : コントロールからパラメータの取得とチェック
			if (_tbCensorBlockSize.GetValue() != 0)
			{
				if (IsDivisible(Trimmed.size(), _tbCensorBlockSize.GetValue()))
				{
					iCensorBlockSize = _tbCensorBlockSize.GetValue();
				}
			}
			else
			{
				iCensorBlockSize = 1;
			}
			if (_tbChessboardBlockSize.GetValue() != 0)
			{
				if (IsDivisible(Trimmed.size(), _tbChessboardBlockSize.GetValue()))
				{
					iChessboardBlockSize = _tbChessboardBlockSize.GetValue();
				}
			}
			else
			{
				iChessboardBlockSize = 1;
			}

			Trimmed = CreateTrimmedDivisibleImage(Source, _MaxCensorBlockSize);
			Censored = CreateCensoredImage(Trimmed, iCensorBlockSize);
			ChessBoard = CreateChessboradPatternImage(Trimmed.size(), iChessboardBlockSize);
			BlendImagesType Blends;
			Blends.push_back(make_pair(_tbSourceBlendRate.GetValue(), Trimmed));
			Blends.push_back(make_pair(_tbCensorBlendRate.GetValue(), Censored));
			Blends.push_back(make_pair(_tbChessboardBlendRate.GetValue(), ChessBoard));
			Blended = BlendImages(Blends);
			cv::GaussianBlur(Blended, Result, Size(5, 5), _tbBlurSigma.GetValue(), _tbBlurSigma.GetValue(), BORDER_DEFAULT);
		}
	}
};

//-------------------------------- public functions --------------------------------

/** インタラクティブGUIを実行する
 */
void censor_reduction::ExecuteInteractiveGui(const cv::Mat& source)
{
	//入出力画像を設定
	_SourceImage = source.clone();
	_ResultImage = source.clone();

	//ウィンドウの用意
	const string AppricationName = "Censor Reduction Tool";
	const string MainWindowName = AppricationName + " / Preview";
	const string ControllWindowName = AppricationName + " / Control";
	namedWindow(MainWindowName, CV_WINDOW_NORMAL);
	namedWindow(ControllWindowName, CV_WINDOW_NORMAL);

	//トラックバーの用意
	_tbSourceBlendRate = CGuiTrackbarFloat("source blend rate", ControllWindowName, 0.f, 1.f);
	_tbCensorBlendRate = CGuiTrackbarFloat("censor blend rate", ControllWindowName, 0.f, 1.f);
	_tbCensorBlockSize = CGuiTrackbarInteger("censor block size", ControllWindowName, _MaxCensorBlockSize);
	_tbChessboardBlendRate = CGuiTrackbarFloat("chessboard blend rate", ControllWindowName, 0.f, 0.1f);
	_tbChessboardBlockSize = CGuiTrackbarInteger("chessboard block size", ControllWindowName, _MaxCensorBlockSize);
	_tbBlurSigma = CGuiTrackbarFloat("blur sigma", ControllWindowName, 0.01f, 10.f);

	//画像
	Mat Result;

	//画像処理スレッドの実行
	_IsTerminate = false;
	thread ImageProcessingThread(&ImageProcessingTHreadHandler);

	for (;;) {
		//キー入力処理
		const int KeyCode = waitKey(33);
		if (KeyCode == 27)
		{
			break;
		}

		//画像処理スレッドから処理結果を取得
		{
			lock_guard<mutex> Lock(_LinkLocalGuard);
			Result = _ResultImage.clone();
		}

		//結果を描画
		imshow(MainWindowName, Result);
	}

	//画像処理スレッドを落とす
	{
		lock_guard<mutex> Lock(_LinkLocalGuard);
		_IsTerminate = true;
	}
	ImageProcessingThread.join();

	cv::imwrite("result.png", Result);
}
