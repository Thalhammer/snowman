#pragma once
#include <stream-itf.h>
#include <string>
#include <vector-wrapper.h>

struct AGC_Instance;
struct NS3_Instance;
namespace snowboy {
	struct OptionsItf;
	struct FrontendStreamOptions {
		std::string ns_power;
		std::string dr_power;
		std::string agc_level;
		std::string agc_power;
		void Register(const std::string&, OptionsItf*);
	};
	struct FrontendStream : StreamItf {
		std::string m_ns_power;
		std::string m_dr_power;
		std::string m_agc_level;
		std::string m_agc_power;
		void* field_x38; // 0x140 bytes
		NS3_Instance* m_ns3_instance;
		AGC_Instance* m_agc_instance;
		Vector field_x50;
		int field_x60;
		int field_x64; // might be padding

		FrontendStream(const FrontendStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~FrontendStream();
	};
} // namespace snowboy