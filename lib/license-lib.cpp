#include <license-lib.h>
#include <snowboy-error.h>

namespace snowboy {
	bool CheckSnowboyLicense() {
		if (!CheckSnowboyLicenseTime())
			throw snowboy_exception{"Your license for Snowboy has been expired. Please contact KITT.AI at snowboy@kitt.ai"};
		if (!CheckSnowboyLicenseCPU() || !CheckSnowboyLicenseSoundCard())
			throw snowboy_exception{"Snowboy license for your device has not been granted. Please contact KITT.AI at snowboy@kitt.ai"};
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
