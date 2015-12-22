# 私用Makefile
#
# 詳しい説明はファイル末尾で
#

#-------------------------------- よくいじる設定 ------------------------------------------------

# 生成されるバイナリファイル名.
BIN_TARGET	:= censor-reduction

# google-perftools
#INCLUDES	+=
#LIBS		+= -ltcmalloc
# boost
#INCLUDES	+= $(BOOST_CXX_FLAGS)
#LIBS		+= $(BOOST_LD_FLAGS) -lboost_program_options -lboost_system -lboost_thread -lboost_date_time -lboost_filesystem -lboost_serialization
# ffmpeg
#INCLUDES	+= $(shell pkg-config libavcodec libavdevice libavfilter libavformat libavutil libswscale --cflags)
#LIBS		+= $(shell pkg-config libavcodec libavdevice libavfilter libavformat libavutil libswscale --libs)
# libpng
#INCLUDES	+= $(shell pkg-config libpng --cflags)
#LIBS		+= $(shell pkg-config libpng --libs)
# eigen3
#INCLUDES	+= $(shell pkg-config eigen3 --cflags)
#LIBS		+= $(shell pkg-config eigen3 --libs)
# OpenCV
INCLUDES	+= $(shell pkg-config opencv --cflags)
LIBS		+= $(shell pkg-config opencv --libs)
# X11
#INCLUDES        += $(shell pkg-config x11 --cflags)
#LIBS            += $(shell pkg-config x11 --libs)
# OpenGL
#INCLUDES        += $(shell pkg-config gl --cflags)
#LIBS            += $(shell pkg-config gl --libs)
# GLU
#INCLUDES        += $(shell pkg-config glu --cflags)
#LIBS            += $(shell pkg-config glu --libs)
# GLEW
#INCLUDES        += $(shell pkg-config glew --cflags)
#LIBS            += $(shell pkg-config glew --libs)
# LabLib
#INCLUDES	+= -I$(HOME)/include
#LIBS		+= -L$(HOME)/lib -lFlow -lGeotools -lIMGobj -lIOTools -lMVobj -lNR -lfilter -lrandom -lrecipes
# CUDA6
#INCLUDES	+= $(shell pkg-config npps-6.0 nppi-6.0 nppc-6.0  cuda-6.0 --cflags)
#LIBS		+= $(shell pkg-config npps-6.0 nppi-6.0 nppc-6.0  cuda-6.0 --libs)
# libboard
#INCLUDES	+= $(LIBBOARD_CXX_FLAGS)
#LIBS		+= $(LIBBOARD_LD_FLAGS)
# pthread
#INCLUDES	+=
#LIBS		+= -lpthread

#-------------------------------- コンパイル設定 --------------------------------

# 使用するgcc
GCC_VERSION	:= g++

# リリースモードで使用する最適化レベル
GCC_REL_OPT		:= -O3

# デバッグオプション
GCC_DBG_OPT		:= -ggdb3

# CUDAターゲットアーキテクチャ
CUDA_ARCH	:= -arch=sm_30

# C++コンパイル情報
CXX					:= $(GCC_VERSION) -MP -MMD -pipe -march=native -std=c++1y
CXX_DEBUG_FLAGS		:= -Wall -W -O0 $(GCC_DBG_OPT) -DDEBUG
CXX_RELEASE_FLAGS	:= $(GCC_REL_OPT) -DNDEBUG
CXX_PROF_FLAGS		:= $(CXX_RELEASE_FLAGS) $(GCC_DBG_OPT) -fno-inline -DGOOGLE_PROFILE

# CUDAコンパイル情報
NVCC				:= nvcc -x cu $(CUDA_ARCH)
NVCC_DEBUG_FLAGS	:= -O0 -g -G -DDEBUG
NVCC_RELEASE_FLAGS	:= -O -DNDEBUG
NVCC_PROF_FLAGS		:= $(NVCC_RELEASE_FLAGS) -pg

# リンク情報
LD					:= $(GCC_VERSION)
LD_DEBUG_FLAGS		:=
LD_RELEASE_FLAGS	:=
LD_PROF_FLAGS		:= $(LD_RELEASE_FLAGS) -lprofiler

#-------------------------------- 場合によっては変更が必要な設定 --------------------------------

# プリコンパイルヘッダ生成用ソースをすべて列挙
PCH_HPP				:= $(shell find . -name  "stdafx.hpp" | xargs realpath)
PCH_GCH				:= $(join $(dir $(PCH_HPP)), $(addsuffix .gch/, $(addprefix ., $(notdir $(PCH_HPP)))))

# ソースファイルから必要なファイルをすべて列挙
SRCS				:= $(shell find . -name "*.cpp" -or -name "*.cu" | xargs realpath)
OBJS				:= $(subst .cu,.cuo,$(subst .cpp,.o,$(SRCS)))
DEPENDS				:= $(addprefix .$(CONF_NAME)/,$(addsuffix .d,$(SRCS)))

# 中間ファイル格納用ディレクトリ名
DEBUG_BUILD_DIR		:=.debug/
RELEASE_BUILD_DIR	:=.release/
PROF_BUILD_DIR		:=.prof/

# 各コンフィグ名
DEBUG_CONF_NAME		:=debug
RELEASE_CONF_NAME	:=release
PROF_CONF_NAME		:=prof

# 入れ子make情報
SUBMAKE_FLAGS		:= -j$(shell grep "processor" /proc/cpuinfo | wc -l) --no-print-directory

#-------------------------------- phonyターゲット --------------------------------

# 未指定の場合は何もしない
.PHONY			: none
none			: ;

# 最適化の状態の問い合わせ
.PHONY			: optimize-state
optimize-state	:
	$(GCC_VERSION) -Q $(REL_OPT) --help=optimizers
	$(GCC_VERSION) -E -v -march=native - 2>&1 | grep cc1

# clean
.PHONY			: clean
clean			:
	rm -f $(BIN_TARGET) $(LIB_TARGET)
	rm -rf $(DEBUG_BUILD_DIR) $(RELEASE_BUILD_DIR) $(PROF_BUILD_DIR)
	rm -rf $(shell find . -follow -name "*.hpp.gch")

# allの場合はdebugですべてビルド
.PHONY			: all
all				: SUBMAKE_FLAGS+=CXX_CONF_FLAGS:="$(CXX_DEBUG_FLAGS)"
all				: SUBMAKE_FLAGS+=LD_CONF_FLAGS:="$(LD_DEBUG_FLAGS)"
all				: SUBMAKE_FLAGS+=NVCC_CONF_FLAGS:="$(NVCC_DEBUG_FLAGS)"
all 			: SUBMAKE_FLAGS+=BUILD_DIR:="$(DEBUG_BUILD_DIR)"
all				: SUBMAKE_FLAGS+=CONF_NAME:="$(DEBUG_CONF_NAME)"
all				: BUILD_DIR:=$(DEBUG_BUILD_DIR)
all				: CONF_NAME:=$(DEBUG_CONF_NAME)
all				: pch
all				: binary

# all-releaseの場合はreleaseですべてビルド
.PHONY			: all-release
all-release		: SUBMAKE_FLAGS+=CXX_CONF_FLAGS:="$(CXX_RELEASE_FLAGS)"
all-release		: SUBMAKE_FLAGS+=LD_CONF_FLAGS:="$(LD_RELEASE_FLAGS)"
all-release		: SUBMAKE_FLAGS+=NVCC_CONF_FLAGS:="$(NVCC_RELEASE_FLAGS)"
all-release		: SUBMAKE_FLAGS+=BUILD_DIR:="$(RELEASE_BUILD_DIR)"
all-release		: SUBMAKE_FLAGS+=CONF_NAME:="$(RELEASE_CONF_NAME)"
all-release		: BUILD_DIR=$(RELEASE_BUILD_DIR)
all-release		: CONF_NAME=$(RELEASE_CONF_NAME)
all-release		: pch
all-release		: binary

# all-profの場合はprofilingですべてビルド
.PHONY			: all-prof
all-prof		: SUBMAKE_FLAGS+=CXX_CONF_FLAGS:="$(CXX_PROF_FLAGS)"
all-prof		: SUBMAKE_FLAGS+=LD_CONF_FLAGS:="$(LD_PROF_FLAGS)"
all-prof		: SUBMAKE_FLAGS+=NVCC_CONF_FLAGS:="$(NVCC_PROF_FLAGS)"
all-prof		: SUBMAKE_FLAGS+=BUILD_DIR:="$(PROF_BUILD_DIR)"
all-prof		: SUBMAKE_FLAGS+=CONF_NAME:="$(PROF_CONF_NAME)"
all-prof		: BUILD_DIR:=$(PROF_BUILD_DIR)
all-prof		: CONF_NAME:=$(PROF_CONF_NAME)
all-prof		: pch
all-prof		: binary

# 多重makeでプリコンパイルヘッダを生成
.PHONY			: pch
pch				:
	@echo "make precompile header"
	@$(MAKE) $(SUBMAKE_FLAGS) $(addsuffix $(CONF_NAME).hpp.gch,$(PCH_GCH))

# 多重makeで実行ファイルを生成
.PHONY			: binary
binary			:
	@echo "make binary file"
	@$(MAKE) $(SUBMAKE_FLAGS) $(BUILD_DIR)$(BIN_TARGET)
	@-cp $(BUILD_DIR)$(BIN_TARGET) $(BIN_TARGET)

#-------------------------------- makeターゲット --------------------------------

# binary
$(BUILD_DIR)$(BIN_TARGET)		: $(addprefix $(BUILD_DIR),$(OBJS))
	$(LD) -o $@ $(LD_CONF_FLAGS) $(addprefix $(BUILD_DIR),$(OBJS)) $(LIBS)

# precompile header
%.stdafx.hpp.gch/$(CONF_NAME).hpp.gch	: %stdafx.hpp
	@mkdir -p $(dir $@)
	@mkdir -p $(BUILD_DIR)$(dir $<)
	$(CXX) -MF $(BUILD_DIR)$<.d -x c++-header -o $@ $(CXX_CONF_FLAGS) $(INCLUDES) $<

# cpp object
$(BUILD_DIR)%.o					: %.cpp
	@mkdir -p $(BUILD_DIR)$(dir $<)
	$(CXX) -MF $(BUILD_DIR)$<.d -c -o $@ $(CXX_CONF_FLAGS) $(INCLUDES) $<
	@echo "$@:$(dir $<)stdafx.hpp" >> $(BUILD_DIR)$<.d

# CUDA object
$(BUILD_DIR)%.cuo				: %.cu
	@mkdir -p $(BUILD_DIR)$(dir $<)
	$(NVCC) -c -o $@ $(NVCC_CONF_FLAGS) $<
	@echo "$(dir $@)" | tr -d '\012' > $(BUILD_DIR)$<.d
	$(NVCC) -M $< $(NVCC_CONF_FLAGS) | sed -e "s/\.o/\.cuo/" >> $(BUILD_DIR)$<.d

# -MMDオプションで生成される依存関係ファイルをインクルード
-include $(DEPENDS)

#
# 以下解説
#
# カレントよりも下の階層のソースファイル全てを列挙してビルドする
#
# ターゲットを指定しない場合はビルドされないので
#	make all
#	make all-release
#  	make all-prof
# のいずれかを設定すること
# デバッグビルド、リリースビルド、プロファイリングビルド、の順番
#
# デバッグビルドの場合は
#	最適化なし
#	デバッグ情報付加
#	"DEBUG"マクロ定義
#
# リリースビルドの場合は
#	最適化有り
#	"NDEBUG"マクロ定義
#
# プロファイリングビルドの場合は
#	リリースビルドのフラグに加えて-pg
#
# ビルド中間ファイルは隠しディレクトリの
#	.debug
#	.release
#	.prof
# に生成される
#
# ソースファイルの拡張子は.cppと.cuを想定
#
# 追加のライブラリのフラグは"INCLUDES"と"LIBS"に追加すること
# =で代入ではなく+=で追加していく
#
# プリコンパイルヘッダーを使用したい場合は各階層に"stdafx.hpp"を配置すること
# この"stdafx.hpp"内でインクルードされたヘッダはすべてプリコンパイルされる
# 各ソースの先頭で"stdafx.hpp"をインクルードすることで恩恵を得られる
# プリコンパイルが終わってからソースファイルのコンパイルが始まる
#
# -jオプションをつけるとプリコンパイルが終わる前にソースファイルのコンパイルが始まってしまう
# なので、makeの呼び出しで-jを付けてはいけない
# 並列ビルドは多段make内で自動的に行われるので心配はいらない
#
# -MMDオプションにより依存関係はすべて自動生成される
#
# CUDAと共存できる
#
