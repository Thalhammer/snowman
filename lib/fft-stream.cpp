#include <feat-lib.h>
#include <fft-stream.h>
#include <frame-info.h>
#include <matrix-wrapper.h>
#include <snowboy-debug.h>
#include <snowboy-math.h>
#include <snowboy-options.h>
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
			// TODO: m_fft.reset(new SplitRadixFft(options));
		} else {
			SNOWBOY_ERROR() << "FFT method has not been implemented: " << m_options.method;
			return;
		}
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
		if ((res & 0xc2) != 0 || m.m_rows == 0) {
			mat->Resize(0, 0);
			info->clear();
			return res;
		}
		if (num_fft_points == -1) {
			SubVector svec{m, 0};
			// Check if size is a power of two
			if (svec.m_size == 0 || (svec.m_size & (svec.m_size - 1)) != 0) {
				num_fft_points = NearestPowerOfTwoCeil(svec.m_size);
			} else
				num_fft_points = svec.m_size;
			InitFft(num_fft_points);
		}
		mat->Resize(m.m_rows, num_fft_points);
		for (int r = 0; r < m.m_rows; r++) {
			SubVector svec{m, r};
			Vector v;
			v.Resize(svec.m_size, MatrixResizeType::kUndefined);
			v.CopyFromVec(svec);
			if (v.m_size < num_fft_points) {
				// TODO: Cant we correctly size the matrix in the first place ?
				v.Resize(num_fft_points, MatrixResizeType::kCopyData);
			}
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