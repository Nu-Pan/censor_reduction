
#include "stdafx.hpp"

#include "interactive_gui.hpp"

using namespace std;
using namespace cv;
using namespace censor_reduction;

int main(int argc, char* argv[])
{

	//入力画像を用意.デバッグ用.
	Mat Source = imread("images\\b6fcd15cb7e9b6c0b47c37c4efb58efa.png", 1);
	//Mat Source = imread("images\\b23d1be367600da8c5ccc7339fa27213.jpg", 1);
	//Mat Source = imread("images\\sauage_fattner_censored_x8.png", 1);
	//Mat Source = imread("images\\papi_colored_censored_x16.png", 1);
	//Mat Source = imread("20151221230507.png", 1);

	ExecuteInteractiveGui(Source);

	return 0;
}
