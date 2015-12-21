
#include "stdafx.h"

#include "macro_utils.hpp"
#include "opencv_utils.hpp"
#include "mat_image_iterator.hpp"
#include "censor_reduction.hpp"

using namespace std;
using namespace cv;
using namespace censor_reduction;

int main(int argc, char* argv[])
{
	//定数
	const string AppricationName = "Censor Reduction Tool";
	const int MaxCensorBlockSize = 32;

	//入力画像を用意.デバッグ用.
	Mat Source = imread("images\\b6fcd15cb7e9b6c0b47c37c4efb58efa.png", 1);
	//Mat Source = imread("images\\b23d1be367600da8c5ccc7339fa27213.jpg", 1);
	//Mat Source = imread("images\\sauage_fattner_censored_x8.png", 1);
	//Mat Source = imread("images\\papi_colored_censored_x16.png", 1);

	//オリジナル画像を予めトリム
	Mat Trimmed = CreateTrimmedDivisibleImage(Source, MaxCensorBlockSize);

	//XXX 消えるはず : GUIで制御するパラメータ類
	int iCensorBlockSize = 1;
	int iChessboardBlockSize = 1;

	//プレビューウィンドウの用意
	const string MainWindowName = AppricationName + " / Preview";
	namedWindow(MainWindowName, CV_WINDOW_NORMAL);

	//コントロールウィンドウの用意
	const string ControllWindowName = AppricationName + " / Controll";
	namedWindow(ControllWindowName, CV_WINDOW_NORMAL);
	CGuiTrackbarFloat tbSourceBlendRate("source_blend_rate", ControllWindowName, 0.f, 1.f);
	CGuiTrackbarFloat tbCensorBlendRate("censor_blend_rate", ControllWindowName, 0.f, 1.f);
	CGuiTrackbarInteger tbCensorBlockSize("censor_block_size", ControllWindowName, MaxCensorBlockSize);
	CGuiTrackbarFloat tbChessboardBlendRate("chessboard_blend_rate", ControllWindowName, 0.f, 0.1f);
	CGuiTrackbarInteger tbChessboardBlockSize("chessboard_block_size", ControllWindowName, MaxCensorBlockSize);
	CGuiTrackbarFloat tbBlurSigma("blur_sigma", ControllWindowName, 0.01f, 10.f);

	//XXX デバッグ用
	Mat Blured;

	for (;;) {
		//キー入力処理
		const int KeyCode = waitKey(33);
		if (KeyCode == 27)
		{
			break;
		}

		//XXX トラックバーラッパーでなんとかするべき : コントロールからパラメータの取得とチェック
		if (tbCensorBlockSize.GetValue() != 0)
		{
			if (IsDivisible(Trimmed.size(), tbCensorBlockSize.GetValue()))
			{
				iCensorBlockSize = tbCensorBlockSize.GetValue();
			}
		}
		else
		{
			iCensorBlockSize = 1;
		}
		if (tbChessboardBlockSize.GetValue() != 0)
		{
			if (IsDivisible(Trimmed.size(), tbChessboardBlockSize.GetValue()))
			{
				iChessboardBlockSize = tbChessboardBlockSize.GetValue();
			}
		}
		else
		{
			iChessboardBlockSize = 1;
		}

		//画像処理の実行
		Mat Censored = CreateCensoredImage(Trimmed, iCensorBlockSize);
		Mat ChessBoard = CreateChessboradPatternImage(Trimmed.size(), iChessboardBlockSize);
		BlendImagesType Blends;
		Blends.push_back(make_pair(tbSourceBlendRate.GetValue(), Trimmed));
		Blends.push_back(make_pair(tbCensorBlendRate.GetValue(), Censored));
		Blends.push_back(make_pair(tbChessboardBlendRate.GetValue(), ChessBoard));
		Mat Blended = BlendImages(Blends);
		cv::GaussianBlur(Blended, Blured, Size(5, 5), tbBlurSigma.GetValue(), tbBlurSigma.GetValue(), BORDER_DEFAULT);

		//結果を描画
		imshow(MainWindowName, Blured);
	}
	
	cv::imwrite("result.png", Blured);

	return 0;
}
