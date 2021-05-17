#pragma once
#include <deque>
#include <matrix-wrapper.h>
#include <memory>
#include <nnet-lib.h>
#include <stream-itf.h>
#include <string>

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

	struct UniversalDetectStream : StreamItf {
		struct PieceInfo {
			char unknown[12];
		};

		UniversalDetectStreamOptions m_options;
		int field_x58;
		int field_x5c;
		bool field_x60;
		int field_x64;
		bool field_x68;
		int field_x6c;

		struct KeyWordInfo {
			// Kw <= unsure what this means
			std::vector<int> field_x88;
			// Kw Sensitivity
			float sensitivity;
			// Kw High Sensitivity
			float high_sensitivity;
			int hotword_id;
			// Kw Search Method
			int search_method;  // TODO: This could be an enum
			// Kw Search Neighbour
			int search_neighbour;
			// Kw DurationPass
			int duration_pass;
			// Kw FloorPass
			int floor_pass;
			// Kw SearchMask
			std::vector<int> search_mask;
			// Kw SearchFloor
			std::vector<float> search_floor;
			// Kw SearchMax
			bool search_max;
			int field_x1c0;
			// Kw NumPieces
			int field_x1d8;
			bool field_x280;
			int field_x298;

			void ReadKeyword(bool binary, std::istream* is, int slide_window);
			void WriteKeyword(bool binary, std::ostream* os) const;
		};

		struct ModelInfo {
			Nnet network;
			std::vector<KeyWordInfo> keywords;
			// License start
			long license_start;
			// License days
			float license_days;

			std::vector<std::vector<std::vector<PieceInfo>>> field_x1f0;
			// Smooth window
			int smooth_window;
			// Slide window
			int slide_window;
			std::vector<std::deque<float>> field_x238;
			std::vector<std::deque<float>> field_x250;
			std::vector<float> field_x268;
			std::vector<float> field_x2b0;

			void CheckLicense() const;
			void SmoothPosterior(Matrix* param_2);
			float HotwordNaiveSearch(int) const;
			int NumHotwords() const;
			void ReadHotwordModel(bool binary, std::istream* is, int num_repeats, int* hotword_id);
			void WriteHotwordModel(bool binary, std::ostream* os) const;
			void ResetDetection();
			void UpdateLicense(long, float);
		};

		std::vector<ModelInfo> m_model_info;

		UniversalDetectStream(const UniversalDetectStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~UniversalDetectStream();

		float GetHotwordPosterior(int, int, int);
		std::string GetSensitivity() const;
		float HotwordDtwSearch(int, int) const;
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
		void UpdateLicense(int, long, float);
		void UpdateModel() const;
		void WriteHotwordModel(bool binary, const std::string& filename) const;
	};
} // namespace snowboy