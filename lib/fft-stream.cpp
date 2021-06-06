#include <feat-lib.h>
#include <fft-stream.h>
#include <frame-info.h>
#include <matrix-wrapper.h>
#include <snowboy-error.h>
#include <snowboy-math.h>
#include <snowboy-options.h>
#include <srfft.h>
#include <vector-wrapper.h>

namespace snowboy {
	void FftStreamOptions::Register(const std::string& prefix, OptionsItf* options) {
		options->Register(prefix, "num-fft-points", "Number of fft points.", &num_fft_points);
		options->Register(prefix, "method", "Specify what FFT method to be used. Possible implementations are \"fft\" and \"srfft\".", &method);
	}

	void FftStream::InitFft(int num_points) {
		FftOptions options;
		options.field_x00 = true;
		options.num_fft_points = num_points;
		// TODO: We haven't reversed srfft yet and fft should give the same result (but slower)
		m_options.method = "fft";
		if (m_options.method == "fft") {
			// Never used in any of my models
			m_fft.reset(new Fft(options));
		} else if (m_options.method == "srfft") {
			m_fft.reset(new SplitRadixFft(options));
		} else
			throw snowboy_exception{"FFT method has not been implemented: " + m_options.method};
	}

	FftStream::FftStream(const FftStreamOptions& options) {
		m_options = options;
		num_fft_points = m_options.num_fft_points;
		if (num_fft_points != -1) {
			InitFft(num_fft_points);
		}
	}

	int FftStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix m;
		auto res = m_connectedStream->Read(&m, info);
		if ((res & 0xc2) != 0 || m.rows() == 0) {
			mat->Resize(0, 0);
			info->clear();
			return res;
		}
		if (num_fft_points == -1) {
			SubVector svec{m, 0};
			// Check if size is a power of two
			if (svec.size() == 0 || (svec.size() & (svec.size() - 1)) != 0) {
				num_fft_points = NearestPowerOfTwoCeil(svec.size());
			} else
				num_fft_points = svec.size();
			InitFft(num_fft_points);
		}
		mat->Resize(m.rows(), num_fft_points);
		Vector v;
		for (size_t r = 0; r < m.rows(); r++) {
			v.Resize(std::max<size_t>(m.cols(), num_fft_points));
			v.CopyFromVec(SubVector{m, r});
			m_fft->DoFft(&v);
			// TODO: Cant we work directly on mat ?
			SubVector{*mat, r}.CopyFromVec(v);
		}
		return res;
	}

	bool FftStream::Reset() {
		m_fft.reset();
		num_fft_points = -1;
		return true;
	}

	std::string FftStream::Name() const {
		return "FftStream";
	}

	FftStream::~FftStream() {}
} // namespace snowboy
