#include <license-lib.h>
#include <snowboy-debug.h>

namespace snowboy {
	bool CheckSnowboyLicense() {
		if (!CheckSnowboyLicenseTime()) {
			SNOWBOY_ERROR() << "Your license for Snowboy has been expired. Please contact KITT.AI at snowboy@kitt.ai";
			return false;
		}
		if (!CheckSnowboyLicenseCPU() || !CheckSnowboyLicenseSoundCard()) {
			SNOWBOY_ERROR() << "Snowboy license for your device has not been granted. Please contact KITT.AI at snowboy@kitt.ai";
			return false;
		}
		return true;
	}

	bool CheckSnowboyLicenseCPU() {
		// NOTE: This was actually how it was implemented, this is not a bypass
		return true;
	}

	bool CheckSnowboyLicenseSoundCard() {
		// NOTE: This was actually how it was implemented, this is not a bypass
		return true;
	}

	bool CheckSnowboyLicenseTime() {
		// NOTE: This was actually how it was implemented, this is not a bypass
		return true;
	}
} // namespace snowboy