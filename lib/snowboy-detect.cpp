#include <audio-lib.h>
#include <matrix-wrapper.h>
#include <memory>
#include <pipeline-detect.h>
#include <pipeline-personal-enroll.h>
#include <pipeline-template-cut.h>
#include <pipeline-vad.h>
#include <snowboy-detect.h>
#include <snowboy-error.h>
#include <wave-header.h>

namespace snowboy {
	SnowboyDetect::SnowboyDetect(const std::string& resource_filename, const std::string& model_str) {
		PipelineDetectOptions options{};
		options.applyFrontend = false;
		options.sampleRate = 16000;
		detect_pipeline_.reset(new PipelineDetect{options});
		detect_pipeline_->SetResource(resource_filename);
		detect_pipeline_->SetModel(model_str);
		detect_pipeline_->Init();

		wave_header_.reset(new WaveHeader{});
		wave_header_->dwSamplesPerSec = detect_pipeline_->GetPipelineSampleRate();
		detect_pipeline_->SetMaxAudioAmplitude(GetMaxWaveAmplitude(*wave_header_));
	}

	SnowboyDetect::~SnowboyDetect() {
		wave_header_.reset();
		detect_pipeline_.reset();
	}

	bool SnowboyDetect::Reset() {
		detect_pipeline_->Reset();
		return true;
	}

	int SnowboyDetect::RunDetection(const std::string& data, bool is_end) {
		if ((data.size() % wave_header_->wBlockAlign) != 0) return -1;
		Matrix data_mat;
		ReadRawWaveFromString(*wave_header_, data, &data_mat);
		return detect_pipeline_->RunDetection(data_mat, is_end);
	}

	int SnowboyDetect::RunDetection(const float* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyDetect: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		mat.Scale(GetMaxWaveAmplitude(*wave_header_));
		return detect_pipeline_->RunDetection(mat, is_end);
	}

	int SnowboyDetect::RunDetection(const int16_t* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyDetect: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return detect_pipeline_->RunDetection(mat, is_end);
	}

	int SnowboyDetect::RunDetection(const int32_t* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyDetect: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return detect_pipeline_->RunDetection(mat, is_end);
	}

	void SnowboyDetect::SetSensitivity(const std::string& sensitivity_str) {
		detect_pipeline_->SetSensitivity(sensitivity_str);
	}

	void SnowboyDetect::SetHighSensitivity(const std::string& high_sensitivity_str) {
		detect_pipeline_->SetHighSensitivity(high_sensitivity_str);
	}

	std::string SnowboyDetect::GetSensitivity() const {
		return detect_pipeline_->GetSensitivity();
	}

	void SnowboyDetect::SetAudioGain(const float audio_gain) {
		detect_pipeline_->SetAudioGain(audio_gain);
	}

	void SnowboyDetect::UpdateModel() const {
		detect_pipeline_->UpdateModel();
	}

	int SnowboyDetect::NumHotwords() const {
		return detect_pipeline_->NumHotwords();
	}

	void SnowboyDetect::ApplyFrontend(const bool apply_frontend) {
		detect_pipeline_->ApplyFrontend(apply_frontend);
	}

	int SnowboyDetect::SampleRate() const {
		return wave_header_->dwSamplesPerSec;
	}

	int SnowboyDetect::NumChannels() const {
		return wave_header_->wChannels;
	}

	int SnowboyDetect::BitsPerSample() const {
		return wave_header_->wBitsPerSample;
	}

	SnowboyVad::SnowboyVad(const std::string& resource_filename) {
		PipelineVadOptions options{};
		options.applyFrontend = false;
		options.sampleRate = 16000;
		vad_pipeline_.reset(new PipelineVad{options});
		vad_pipeline_->SetResource(resource_filename);
		vad_pipeline_->Init();

		wave_header_.reset(new WaveHeader{});
		wave_header_->dwSamplesPerSec = vad_pipeline_->GetPipelineSampleRate();
		vad_pipeline_->SetMaxAudioAmplitude(GetMaxWaveAmplitude(*wave_header_));
	}

	SnowboyVad::~SnowboyVad() {
		wave_header_.reset();
		vad_pipeline_.reset();
	}

	bool SnowboyVad::Reset() {
		vad_pipeline_->Reset();
		return true;
	}

	int SnowboyVad::RunVad(const std::string& data, bool is_end) {
		if ((data.size() % wave_header_->wBlockAlign) != 0) return -1;
		Matrix data_mat;
		ReadRawWaveFromString(*wave_header_, data, &data_mat);
		return vad_pipeline_->RunVad(data_mat, is_end);
	}

	int SnowboyVad::RunVad(const float* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyVad: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		mat.Scale(GetMaxWaveAmplitude(*wave_header_));
		return vad_pipeline_->RunVad(mat, is_end);
	}

	int SnowboyVad::RunVad(const int16_t* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyVad: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return vad_pipeline_->RunVad(mat, is_end);
	}

	int SnowboyVad::RunVad(const int32_t* const data, const int array_length, bool is_end) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyVad: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return vad_pipeline_->RunVad(mat, is_end);
	}

	void SnowboyVad::SetAudioGain(const float audio_gain) {
		vad_pipeline_->SetAudioGain(audio_gain);
	}

	void SnowboyVad::ApplyFrontend(const bool apply_frontend) {
		vad_pipeline_->ApplyFrontend(apply_frontend);
	}

	int SnowboyVad::SampleRate() const {
		return wave_header_->dwSamplesPerSec;
	}

	int SnowboyVad::NumChannels() const {
		return wave_header_->wChannels;
	}

	int SnowboyVad::BitsPerSample() const {
		return wave_header_->wBitsPerSample;
	}

	SnowboyPersonalEnroll::SnowboyPersonalEnroll(const std::string& resource_filename, const std::string& model_filename) {
		PipelinePersonalEnrollOptions options{};
		options.sample_rate = 16000;
		enroll_pipeline_.reset(new PipelinePersonalEnroll{options});
		enroll_pipeline_->SetResource(resource_filename);
		enroll_pipeline_->SetModelFilename(model_filename);
		enroll_pipeline_->Init();

		wave_header_.reset(new WaveHeader{});
		wave_header_->dwSamplesPerSec = enroll_pipeline_->GetPipelineSampleRate();
	}

	int SnowboyPersonalEnroll::RunEnrollment(const std::string& data) {
		if ((data.size() % wave_header_->wBlockAlign) != 0) return -1;
		Matrix mat;
		ReadRawWaveFromString(*wave_header_, data, &mat);
		return RunEnrollment(mat);
	}

	int SnowboyPersonalEnroll::RunEnrollment(const float* const data, const int array_length) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		mat.Scale(GetMaxWaveAmplitude(*wave_header_));
		return RunEnrollment(mat);
	}

	int SnowboyPersonalEnroll::RunEnrollment(const int16_t* const data, const int array_length) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return RunEnrollment(mat);
	}

	int SnowboyPersonalEnroll::RunEnrollment(const int32_t* const data, const int array_length) {
		if (data == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data is NULL"};
		Matrix mat;
		mat.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		// No idea if this is correct, but it looks right...
		for (size_t c = 0; c < mat.cols(); c++)
		{
			for (size_t r = 0; r < mat.rows(); r++)
			{
				mat(r, c) = data[c * mat.rows() + r];
			}
		}
		return RunEnrollment(mat);
	}

	int SnowboyPersonalEnroll::RunEnrollment(const MatrixBase& data) {
		auto res = enroll_pipeline_->RunEnrollment(data);
		if ((res & 2) == 0) {
			if ((res & 0x200) == 0) return (res >> 9) & 2;
			return 1;
		}
		return -1;
	}

	bool SnowboyPersonalEnroll::Reset() {
		enroll_pipeline_->Reset();
		return true;
	}

	int SnowboyPersonalEnroll::GetNumTemplates() const {
		return enroll_pipeline_->GetNumTemplates();
	}

	int SnowboyPersonalEnroll::SampleRate() const {
		return wave_header_->dwSamplesPerSec;
	}

	int SnowboyPersonalEnroll::NumChannels() const {
		return wave_header_->wChannels;
	}

	int SnowboyPersonalEnroll::BitsPerSample() const {
		return wave_header_->wBitsPerSample;
	}

	SnowboyPersonalEnroll::~SnowboyPersonalEnroll() {}

	SnowboyTemplateCut::SnowboyTemplateCut(const std::string& resource_filename) {
		PipelineTemplateCutOptions options{};
		options.sample_rate = 16000;
		options.min_non_voice_frames = 20;
		options.min_voice_frames = 20;
		options.bg_energy_threshold = 2.0f;
		cut_pipeline_.reset(new PipelineTemplateCut{options});
		cut_pipeline_->SetResource(resource_filename);
		cut_pipeline_->Init();
		wave_header_.reset(new WaveHeader{});
		wave_header_->dwSamplesPerSec = cut_pipeline_->GetPipelineSampleRate();
	}

	std::string SnowboyTemplateCut::CutTemplate(const std::string& data) {
		if ((data.size() % wave_header_->wBlockAlign) != 0) return {};
		Matrix mat_data, mat_out;
		ReadRawWaveFromString(*wave_header_, data, &mat_data);
		auto res = cut_pipeline_->CutTemplate(mat_data, &mat_out);
		std::string ret{};
		if ((res & 2) == 0) WriteRawWaveToString(*wave_header_, mat_out, &ret);
		return ret;
	}

	int SnowboyTemplateCut::CutTemplate(const float* const data, const int array_length, float* const data_out, int* array_length_out) {
		if (data == nullptr || data_out == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data or data_out is NULL"};
		Matrix mat_data, mat_out;
		mat_data.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		for (size_t c = 0; c < mat_data.cols(); c++)
			for (size_t r = 0; r < mat_data.rows(); r++)
				mat_data(r, c) = data[c * mat_data.rows() + r];
		mat_data.Scale(GetMaxWaveAmplitude(*wave_header_));
		auto res = cut_pipeline_->CutTemplate(mat_data, &mat_out);
		if ((res & 2) == 0) {
			for (size_t c = 0; c < mat_out.cols(); c++)
				for (size_t r = 0; r < mat_out.rows(); r++)
					data_out[c * mat_out.rows() + r] = mat_out(r, c);
			if (array_length_out) *array_length_out = mat_out.rows() * mat_out.cols();
			return 0;
		}
		return 1;
	}

	int SnowboyTemplateCut::CutTemplate(const int16_t* const data, const int array_length, int16_t* const data_out, int* array_length_out) {
		if (data == nullptr || data_out == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data or data_out is NULL"};
		Matrix mat_data, mat_out;
		mat_data.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		for (size_t c = 0; c < mat_data.cols(); c++)
			for (size_t r = 0; r < mat_data.rows(); r++)
				mat_data(r, c) = data[c * mat_data.rows() + r];
		auto res = cut_pipeline_->CutTemplate(mat_data, &mat_out);
		if ((res & 2) == 0) {
			for (size_t c = 0; c < mat_out.cols(); c++)
				for (size_t r = 0; r < mat_out.rows(); r++)
					data_out[c * mat_out.rows() + r] = mat_out(r, c);
			if (array_length_out) *array_length_out = mat_out.rows() * mat_out.cols();
			return 0;
		}
		return 1;
	}

	int SnowboyTemplateCut::CutTemplate(const int32_t* const data, const int array_length, int32_t* const data_out, int* array_length_out) {
		if (data == nullptr || data_out == nullptr)
			throw snowboy_exception{"SnowboyPersonalEnroll: data or data_out is NULL"};
		Matrix mat_data, mat_out;
		mat_data.Resize(wave_header_->wChannels, array_length / wave_header_->wChannels, MatrixResizeType::kSetZero);
		for (size_t c = 0; c < mat_data.cols(); c++)
			for (size_t r = 0; r < mat_data.rows(); r++)
				mat_data(r, c) = data[c * mat_data.rows() + r];
		mat_data.Scale(GetMaxWaveAmplitude(*wave_header_));
		auto res = cut_pipeline_->CutTemplate(mat_data, &mat_out);
		if ((res & 2) == 0) {
			for (size_t c = 0; c < mat_out.cols(); c++)
				for (size_t r = 0; r < mat_out.rows(); r++)
					data_out[c * mat_out.rows() + r] = mat_out(r, c);
			if (array_length_out) *array_length_out = mat_out.rows() * mat_out.cols();
			return 0;
		}
		return 1;
	}

	bool SnowboyTemplateCut::Reset() {
		cut_pipeline_->Reset();
		return true;
	}

	int SnowboyTemplateCut::SampleRate() const {
		return wave_header_->dwSamplesPerSec;
	}

	int SnowboyTemplateCut::NumChannels() const {
		return wave_header_->wChannels;
	}

	int SnowboyTemplateCut::BitsPerSample() const {
		return wave_header_->wBitsPerSample;
	}

	SnowboyTemplateCut::~SnowboyTemplateCut() {}

} // namespace snowboy
