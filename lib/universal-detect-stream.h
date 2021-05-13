#pragma once
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>
#include <string>
#include <deque>

namespace snowboy {
	struct OptionsItf;
    struct Nnet;

	struct UniversalDetectStreamOptions {
		int slide_step;
		int min_num_frames_per_phone;
		int num_repeats;
		unsigned int min_detection_interval;
		std::string sensitivity_str;
		std::string high_sensitivity_str;
		std::string model_str;
		std::string smooth_window_str;
		std::string slide_window_str;
		bool debug_mode;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(UniversalDetectStreamOptions) == 0x40);

	struct UniversalDetectStream : StreamItf {
        struct PieceInfo {
            char unknown[12];
        };
        static_assert(sizeof(PieceInfo) == 0xc);

        UniversalDetectStreamOptions m_options;
        int field_x58;
        int field_x5c;
        bool field_x60;
        int field_x64;
        bool field_x68;
        int field_x6c;

        // TODO: This really needs a refactor asap once we can...
        // Mommy, I am scared...
        // Whoever though that 25 vectors (of vectors of vectors...) was
        // a good idea instead of just putting them into an object
        // or at least a struct should never touch a computer again.
        // And for goods sake no C++ code....
        std::vector<Nnet> field_x70;
        // Kw <= unsure what this means
        std::vector<std::vector<std::vector<int>>> field_x88;
        // Kw Sensitivity
        std::vector<std::vector<float>> field_xa0;
        // Kw High Sensitivity
        std::vector<std::vector<float>> field_xb8;
        std::vector<std::vector<int>> field_xd0;
        // Kw Search Method
        std::vector<std::vector<int>> field_xe8;
        // Kw Search Neighbour
        std::vector<std::vector<int>> field_x100;
        // Kw DurationPass
        std::vector<std::vector<int>> field_x118;
        // Kw FloorPass
        std::vector<std::vector<int>> field_x130;
        // Kw SearchMask
        std::vector<std::vector<std::vector<int>>> field_x148;
        // Kw SearchFloor
        std::vector<std::vector<std::vector<float>>> field_x160;
        // Kw SearchMax
        std::vector<std::vector<bool>> field_x178;
        // License start
        std::vector<long> field_x190;
        // License days
        std::vector<float> field_x1a8;
        std::vector<std::vector<int>> field_x1c0;
        // Kw NumPieces
        std::vector<std::vector<int>> field_x1d8;
        std::vector<std::vector<std::vector<std::vector<PieceInfo>>>> field_x1f0;
        // Smooth window
        std::vector<int> field_x208;
        // Slide window
        std::vector<int> field_x220;
        std::vector<std::vector<std::deque<float>>> field_x238;
        std::vector<std::vector<std::deque<float>>> field_x250;
        std::vector<std::vector<float>> field_x268;
        std::vector<std::vector<bool>> field_x280;
        std::vector<std::vector<int>> field_x298;
        std::vector<std::vector<float>> field_x2b0;

		UniversalDetectStream(const UniversalDetectStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~UniversalDetectStream();

		void CheckLicense(int) const;
		float GetHotwordPosterior(int, int, int);
		std::string GetSensitivity() const;
		float HotwordDtwSearch(int, int) const;
		float HotwordNaiveSearch(int, int) const;
		float HotwordPiecewiseSearch(int, int) const;
		float HotwordViterbiSearch(int, int) const;
		float HotwordViterbiSearch(int, int, int, const PieceInfo&) const;
        float HotwordViterbiSearchReduplication(int, int, int);
        float HotwordViterbiSearchSoftFloor(int, int) const;
        float HotwordViterbiSearchTraceback(int, int) const;
        float HotwordViterbiSearchTracebackLog(int, int) const;
        int NumHotwords(int) const;
        void PushSlideWindow(int, const MatrixBase&);
        void ReadHotwordModel(const std::string& filename);
        void ResetDetection();
        void SetHighSensitivity(const std::string&);
        void SetSensitivity(const std::string&);
        void SetSlideWindowSize(const std::string&);
        void SetSmoothWindowSize(const std::string&);
        void SmoothPosterior(int, Matrix*);
        void UpdateLicense(int, long, float);
        void UpdateModel() const;
        void WriteHotwordModel(bool binary, const std::string& filename) const;
	};
	static_assert(sizeof(UniversalDetectStream) == 0x2c8);
} // namespace snowboy