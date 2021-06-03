#include <agc.h>
#include <frame-info.h>
#include <frontend-stream.h>
#include <matrix-wrapper.h>
#include <ns3.h>
#include <snowboy-error.h>
#include <snowboy-options.h>

#define ENABLE_FRONTEND_STREAM 0

namespace snowboy {
	void FrontendStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "ns-power", "NS power.", &ns_power);
		opts->Register(prefix, "dr-power", "DR power.", &dr_power);
		opts->Register(prefix, "agc-level", "AGC level.", &agc_level);
		opts->Register(prefix, "agc-power", "AGC power.", &agc_power);
	}

	FrontendStream::FrontendStream(const FrontendStreamOptions& options) {
		m_connectedStream = nullptr;
		m_isConnected = false;
		m_ns_power = options.ns_power;
		m_dr_power = options.dr_power;
		m_agc_level = options.agc_level;
		m_agc_power = options.agc_power;
#if ENABLE_FRONTEND_STREAM
		field_x60 = 0xa0;
		field_x38 = malloc(0x140); // this should be new x, might be 160 shorts
		m_ns3_instance = nullptr;
		m_agc_instance = nullptr;
		try {
			Reset();
		} catch (...) {
			if (m_ns3_instance) NS3_Exit(m_ns3_instance);
			if (m_agc_instance) AGC_Exit(m_agc_instance);
			if (field_x38) free(field_x38); // this should be delete x
			m_ns3_instance = nullptr;
			m_agc_instance = nullptr;
			field_x38 = nullptr;
			throw;
		}
#endif
	}

	int FrontendStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
#if !ENABLE_FRONTEND_STREAM
		// TODO: Implement this stuff
		// It seems to work reasonably good without AGC and Noise suppression,
		// so we just read from the previous stream for now.
		// However for performance and completeness we should really implement it correctly
		return m_connectedStream->Read(mat, info);
#else
		// TODO:
		Matrix m;
		auto res = m_connectedStream->Read(&m, info);
		field_x64 = field_x64 + m.m_cols;
		if (field_x64 > 48000) {
			this->Reset();
		}
		if ((res & 0xc2) == 0 && m.m_rows != 0) {
			Vector v;
			v.Resize(field_x50.size() + m.m_cols);
			v.Range(0, field_x50.size()).CopyFromVec(field_x50);
			v.Range(field_x50.size(), m.m_cols).CopyFromVec(SubVector{m, 0});
			field_x50.Resize(0);
			auto x = v.size() / field_x60;
			mat->Resize(1, x * field_x60);
			// TODO: More code between here
			if (x > 0) {
			}
			// TODO: and here
		} else {
			mat->Resize(0, 0);
			info->clear();
		}
		return res;
#endif
	}

	bool FrontendStream::Reset() {
#if ENABLE_FRONTEND_STREAM
		if (m_ns3_instance) NS3_Exit(m_ns3_instance);
		if (m_agc_instance) AGC_Exit(m_agc_instance);
		field_x60 = 0xa0;
		int status;
		m_ns3_instance = NS3_Init(16000, 0xa0, &status);
		if (status != 1)
			throw snowboy_exception{"Failed to initialize NS."};
		if (NS3_SetPara(m_ns3_instance, "NS_Power", m_ns_power.c_str()) != 1)
			throw snowboy_exception{"Failed to set NS_Power."};
		if (NS3_SetPara(m_ns3_instance, "DR_Power", m_dr_power.c_str()) != 1)
			throw snowboy_exception{"Failed to set DR_Power."};
		m_agc_instance = AGC_Init(16000, field_x60, 1, &status);
		if (status != 1)
			throw snowboy_exception{"Failed to initialize AGC."};
		if (AGC_SetPara(m_agc_instance, "AGC_Level", m_agc_level.c_str()) != 1)
			throw snowboy_exception{"Failed to set AGC_Level."};
		if (AGC_SetPara(m_agc_instance, "AGC_Power", m_agc_power.c_str()) != 1)
			throw snowboy_exception{"Failed to set AGC_Power."};
		field_x64 = 0;
#endif
		return true;
	}

	std::string FrontendStream::Name() const {
		return "FrontendStream";
	}

	FrontendStream::~FrontendStream() {
#if ENABLE_FRONTEND_STREAM
		if (m_ns3_instance) NS3_Exit(m_ns3_instance);
		if (m_agc_instance) AGC_Exit(m_agc_instance);
		if (field_x38) free(field_x38); // this should be delete x
#endif
		m_connectedStream = nullptr;
		m_isConnected = false;
	}
} // namespace snowboy
