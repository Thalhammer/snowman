#pragma once
#include <deque>
#include <matrix-wrapper.h>
#include <stream-itf.h>

struct AGC_Instance;
struct NS3_Instance;
namespace snowboy {
	struct OptionsItf;
	struct RawEnergyVadStreamOptions {
		bool init_bg_energy;
		float bg_energy_threshold;
		float bg_energy_cap;
		uint32_t bg_buffer_size;
		uint32_t raw_buffer_extra;
		void Register(const std::string&, OptionsItf*);
	};
	struct RawEnergyVadStream : StreamItf {
		RawEnergyVadStreamOptions m_options;
		bool field_x2c;
		float m_bg_energy; // might be
		int field_x34;
		std::deque<std::pair<unsigned int, float>> field_x38;
		std::deque<float> field_x88;
		Matrix m_someMatrix;
		std::vector<FrameInfo> field_xf0;

		RawEnergyVadStream(const RawEnergyVadStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~RawEnergyVadStream();

		void InitRawEnergyVad(Matrix*, std::vector<FrameInfo>*);
		void UpdateBackgroundEnergy(const std::vector<FrameInfo>&);
	};
} // namespace snowboy
