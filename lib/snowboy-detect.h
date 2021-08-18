#pragma once
#include <memory>
#include <string>

namespace snowboy {
	namespace testing {
		class Inspector;
	}
	struct WaveHeader;
	class PipelineDetect;
	struct PipelineVad;
	class PipelinePersonalEnroll;
	class PipelineTemplateCut;
	struct MatrixBase;

	/**
	 * \brief Hotword detector class.
	 *
	 * Provides a high level, easy to use way to detect hotwords and do
	 * voice activity detection.
	 */
	class SnowboyDetect {
		friend class testing::Inspector;

	public:
		/**
		 * \brief Default constructor
		 *
		 * Constructor that takes a resource file, and a list of hotword models which
		 * are separated by comma. In the case that more than one hotword exist in the
		 * provided models, RunDetection() will return the index of the hotword, if
		 * the corresponding hotword is triggered.
		 *
		 * A personal model can only contain one hotword, but an universal model
		 * may contain multiple hotwords. It is your responsibility to figure
		 * out the index of the hotword. For example, if your model string is
		 * "foo.pmdl,bar.umdl", where foo.pmdl contains hotword x, bar.umdl
		 * has two hotwords y and z, the indices of different hotwords are as
		 * follows: \n
		 * x 1 \n
		 * y 2 \n
		 * z 3 \n
		 *
		 * @param [in]  resource_filename   Filename of resource file.
		 * @param [in]  model_str           A string of multiple hotword models,
		 *                                  separated by comma.
		 */
		SnowboyDetect(const std::string& resource_filename,
					  const std::string& model_str);

		/**
		 * \brief Resets the detection.
		 *
		 * This class handles voice activity detection (VAD)
		 * internally. But if you have an external VAD, you should call Reset()
		 * whenever you see segment end from your VAD.
		 *
		 * \return true on success\n
		 * 		   false on error
		 */
		bool Reset();

		/**
		 * \brief Runs hotword detection.
		 *
		 * Supported audio format is WAVE (with linear PCM,
		 * 8-bits unsigned integer, 16-bits signed integer or 32-bits signed integer).
		 * See SampleRate(), NumChannels() and BitsPerSample() for the required
		 * sampling rate, number of channels and bits per sample values. You are
		 * supposed to provide a small chunk of data (e.g., 0.1 second) each time you
		 * call RunDetection(). Larger chunk usually leads to longer delay, but less
		 * CPU usage.
		 *
		 * Definition of return values:
		 * -2: Silence.
		 * -1: Error.
		 *  0: No event.
		 *  1: Hotword 1 triggered.
		 *  2: Hotword 2 triggered.
		 *  ...
		 *
		 * \param [in]  data      Small chunk of data to be detected. See
		 *                        above for the supported data format.
		 * \param [in]  is_end    Set it to true if it is the end of a
		 *                        utterance or file.
		 * \return \n
		 * Code | Info
		 * -----|-----
		 * -2   | Silence.
		 * -1   | Error.
		 *  0   | No event.
		 *  1   | Hotword 1 triggered.
		 *  2   | Hotword 2 triggered.
		 *  ... | Hotword n triggered.
		 */
		int RunDetection(const std::string& data, bool is_end = false);

		/**
		 * \brief Runs hotword detection on float samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunDetection(const std::string&, bool) for details
		 */
		int RunDetection(const float* const data,
						 const int array_length, bool is_end = false);

		/**
		 * \brief Runs hotword detection on int16_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunDetection(const std::string&, bool) for details
		 */
		int RunDetection(const int16_t* const data,
						 const int array_length, bool is_end = false);

		/**
		 * \brief Runs hotword detection on int32_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunDetection(const std::string&, bool) for details
		 */
		int RunDetection(const int32_t* const data,
						 const int array_length, bool is_end = false);

		/**
		 * \brief Sets the sensitivity string for the loaded hotwords.
		 *
		 * A <sensitivity_str> is a list of floating numbers between 0 and 1,
		 * and separated by comma. For example, if there are 3 loaded hotwords,
		 * your string should looks something like this:\n
		 *   0.4,0.5,0.8\n
		 * Make sure you properly align the sensitivity value to the corresponding
		 * hotword.
		 *
		 * \param [in] sensitivity_str		List of sensitivity values.
		 */
		void SetSensitivity(const std::string& sensitivity_str);

		/**
		 * \brief Sets the high sensitivity string for the loaded hotwords.
		 *
		 * Similar to the sensitivity setting above. When set higher than the above
		 * sensitivity, the algorithm automatically chooses between the normal
		 * sensitivity set above and the higher sensitivity set here, to maximize the
		 * performance. By default, it is not set, which means the algorithm will
		 * stick with the sensitivity set above.
		 *
		 * \param [in] high_sensitivity_str List of sensitivity values.
		 */
		void SetHighSensitivity(const std::string& high_sensitivity_str);

		/**
		 * \brief Returns the sensitivity string for the current hotwords.
		 *
		 * \return List of sensitivity values separated by comma
		 */
		std::string GetSensitivity() const;

		/**
		 * \brief Apply a fixed gain to the input audio.
		 *
		 * In case you have a very weak microphone, you can use this function to
		 * boost input audio level.
		 *
		 * \param [in] audio_gain Gain to apply. A gain of 1 means no volume change.
		 */
		void SetAudioGain(const float audio_gain);

		/**
		 * \brief Writes the models to the model filenames specified in <model_str> in the
		 * constructor.
		 *
		 * This overwrites the original model with the latest parameter
		 * setting. You are supposed to call this function if you have updated the
		 * hotword sensitivities through SetSensitivity(), and you would like to store
		 * those values in the model as the default value.
		 */
		void UpdateModel() const;

		/**
		 * \brief Returns the number of the loaded hotwords.
		 *
		 * This helps you to figure the index of the hotwords.
		 *
		 * \return Number of Hotwords currently loaded.
		 */
		int NumHotwords() const;

		/**
		 * \brief Enable or disable audio frontend (NS & AGC).
		 *
		 * If <apply_frontend> is true, then apply frontend audio processing;
		 * otherwise turns the audio processing off. Frontend audio processing
		 * includes algorithms such as automatic gain control (AGC), noise suppression
		 * (NS) and so on. Generally adding frontend audio processing helps the
		 * performance, but if the model is not trained with frontend audio
		 * processing, it may decrease the performance. The general rule of thumb is:
		 *   1. For personal models, set it to false.
		 *   2. For universal models, follow the instruction of each published model
		 *
		 * \param [in] apply_frontend New frontend state
		 */
		void ApplyFrontend(const bool apply_frontend);

		/**
		 * \brief Returns the expected sample rate for audio provided to RunDetection().
		 * \return The expected samplerate.
		 */
		int SampleRate() const;

		/**
		 * \brief Returns the expected number of channels for audio provided to RunDetection().
		 * \return The expected number of channels.
		 */
		int NumChannels() const;

		/**
		 * \brief Returns the expected number of bits for audio provided to RunDetection().
		 * \return The expected number of bits.
		 */
		int BitsPerSample() const;

		/** \brief Destructor */
		~SnowboyDetect();

	private:
		std::unique_ptr<WaveHeader> wave_header_;
		std::unique_ptr<PipelineDetect> detect_pipeline_;
	};

	/**
	 * \brief Voice activity detector class.
	 *
	 * Class that only does voice activity detection
	 * without trying to match any hotwords. Do not use this
	 * in addition to SnowboyDetect, since SnowboyDetect contains
	 * a VAD pipeline as well.
	 */
	class SnowboyVad {
		friend class testing::Inspector;

	public:
		/**
		 * \brief Default constructor
		 *
		 * Constructor that takes a resource file.
		 *
		 * @param [in]  resource_filename   Filename of resource file.
		 */
		SnowboyVad(const std::string& resource_filename);

		/**
		 * \brief Resets the vad.
		 *
		 * After reset the pipeline will behave identical to a freshly constructed instance.
		 *
		 * \return true on success\n
		 * 		   false on error
		 */
		bool Reset();

		/**
		 * \brief Runs the VAD algorithm.
		 *
		 * Supported audio format is WAVE (with linear PCM, 8-bits unsigned integer,
		 * 16-bits signed integer or 32-bits signed integer). See SampleRate(), NumChannels()
		 * and BitsPerSample() for the required sampling rate, number of channels and
		 * bits per sample values. You are supposed to provide a small chunk of data
		 * (e.g., 0.1 second) each time you call RunDetection(). Larger chunk usually
		 * leads to longer delay, but less CPU usage.
		 *
		 * \return \n
		 * Code | Info
		 * -----|-----
		 * -2   | Silence.
		 * -1   | Error.
		 *  0   | No event.
		 *
		 * @param [in]  data               Small chunk of data to be detected. See
		 *                                 above for the supported data format.
		 * @param [in]  is_end             Set it to true if it is the end of a
		 *                                 utterance or file.
		 */
		int RunVad(const std::string& data, bool is_end = false);

		/**
		 * \brief Runs vad on float samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunVad(const std::string&, bool) for details
		 */
		int RunVad(const float* const data,
				   const int array_length, bool is_end = false);

		/**
		 * \brief Runs vad on int16_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunVad(const std::string&, bool) for details
		 */
		int RunVad(const int16_t* const data,
				   const int array_length, bool is_end = false);

		/**
		 * \brief Runs vad on int32_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Small chunk of data to be detected.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \param [in]  is_end             Set it to true if it is the end of a utterance or file.
		 * \return See RunVad(const std::string&, bool) for details
		 */
		int RunVad(const int32_t* const data,
				   const int array_length, bool is_end = false);

		/**
		 * \brief Apply a fixed gain to the input audio.
		 *
		 * In case you have a very weak microphone, you can use this function to
		 * boost input audio level.
		 *
		 * \param [in] audio_gain Gain to apply. A gain of 1 means no volume change.
		 */
		void SetAudioGain(const float audio_gain);

		/**
		 * \brief Enable or disable audio frontend (NS & AGC).
		 *
		 * If <apply_frontend> is true, then apply frontend audio processing;
		 * otherwise turns the audio processing off. Frontend audio processing
		 * includes algorithms such as automatic gain control (AGC), noise suppression
		 * (NS) and so on. Generally adding frontend audio processing helps the
		 * performance, but if the model is not trained with frontend audio
		 * processing, it may decrease the performance. The general rule of thumb is:
		 *   1. For personal models, set it to false.
		 *   2. For universal models, follow the instruction of each published model
		 *
		 * \param [in] apply_frontend New frontend state
		 */
		void ApplyFrontend(const bool apply_frontend);

		/**
		 * \brief Returns the expected sample rate for audio provided to RunDetection().
		 * \return The expected samplerate.
		 */
		int SampleRate() const;

		/**
		 * \brief Returns the expected number of channels for audio provided to RunDetection().
		 * \return The expected number of channels.
		 */
		int NumChannels() const;

		/**
		 * \brief Returns the expected number of bits for audio provided to RunDetection().
		 * \return The expected number of bits.
		 */
		int BitsPerSample() const;

		/** \brief Destructor */
		~SnowboyVad();

	private:
		std::unique_ptr<WaveHeader> wave_header_;
		std::unique_ptr<PipelineVad> vad_pipeline_;
	};

	/**
	 * \brief Enrollment class.
	 *
	 * Class that allows enrolling new personal hotwords.
	 */
	class SnowboyPersonalEnroll {
		friend class testing::Inspector;

	public:
		/**
		 * \brief Default constructor
		 *
		 * Constructor that takes a resource file and a hotword model filename.
		 * The finished model will be written to the specified file.
		 *
		 * @param [in]  resource_filename   Filename of resource file.
		 * @param [in]  model_filename      Filename for the generated hotword model.
		 */
		SnowboyPersonalEnroll(const std::string& resource_filename, const std::string& model_filename);

		/**
		 * \brief Runs the hotword enrollment.
		 *
		 * Supported audio format is WAVE (with linear PCM, 8-bits unsigned integer,
		 * 16-bits signed integer or 32-bits signed integer). See SampleRate(), NumChannels()
		 * and BitsPerSample() for the required sampling rate, number of channels and
		 * bits per sample values. You are supposed to provide a full recording of the hotword
		 * each time you call RunEnrollment().
		 *
		 * \return \n
		 * Code | Info
		 * -----|-----
		 * -1   | Error.
		 *  0   | No event.
		 *  1   | Hotword is too long
		 *  2   | Hotword is too short
		 *
		 * @param [in]  data     Recording of the hotword to enroll.
		 */
		int RunEnrollment(const std::string& data);

		/**
		 * \brief Runs enrollment on float samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording of the hotword to be enrolled.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \return See RunEnrollment(const std::string&) for details
		 */
		int RunEnrollment(const float* const data, const int array_length);

		/**
		 * \brief Runs enrollment on int16_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording of the hotword to be enrolled.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \return See RunEnrollment(const std::string&) for details
		 */
		int RunEnrollment(const int16_t* const data, const int array_length);

		/**
		 * \brief Runs enrollment on int32_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording of the hotword to be enrolled.
		 * \param [in]  array_length       Length of the data array in elements.
		 * \return See RunEnrollment(const std::string&) for details
		 */
		int RunEnrollment(const int32_t* const data, const int array_length);

		/**
		 * \brief Resets the enrollment.
		 *
		 * After reset the pipeline will behave identical to a freshly constructed instance.
		 *
		 * \return true on success\n
		 * 		   false on error
		 */
		bool Reset();

		/**
		 * \brief Returns the minimum number of recordings required for the new hotword.
		 * \return Number of needed recordings.
		 */
		int GetNumTemplates() const;

		/**
		 * \brief Returns the expected sample rate for audio provided to RunDetection().
		 * \return The expected samplerate.
		 */
		int SampleRate() const;

		/**
		 * \brief Returns the expected number of channels for audio provided to RunDetection().
		 * \return The expected number of channels.
		 */
		int NumChannels() const;

		/**
		 * \brief Returns the expected number of bits for audio provided to RunDetection().
		 * \return The expected number of bits.
		 */
		int BitsPerSample() const;

		/** \brief Destructor */
		~SnowboyPersonalEnroll();

	private:
		/**
		 * \brief Runs enrollment on a Matrix
		 * \param [in]  mat		Matrix containing the hotword recording.
		 */
		int RunEnrollment(const MatrixBase& mat);
		std::unique_ptr<WaveHeader> wave_header_;
		std::unique_ptr<PipelinePersonalEnroll> enroll_pipeline_;
	};

	/**
	 * \brief Template cut class.
	 *
	 * Class that uses the voice activity detection to cut a hotword sample to size
	 * by removing all leading and trailing silence.
	 */
	class SnowboyTemplateCut {
		friend class testing::Inspector;

	public:
		/**
		 * \brief Default constructor
		 *
		 * Constructor that takes a resource file.
		 *
		 * @param [in]  resource_filename   Filename of resource file.
		 */
		SnowboyTemplateCut(const std::string& resource_filename);

		/**
		 * \brief Cuts the provided sample.
		 *
		 * Supported audio format is WAVE (with linear PCM, 8-bits unsigned integer,
		 * 16-bits signed integer or 32-bits signed integer). See SampleRate(), NumChannels()
		 * and BitsPerSample() for the required sampling rate, number of channels and
		 * bits per sample values. You are supposed to provide a full recording on each call.
		 *
		 * @param [in]  data    Recording to be cut.
		 * \return The provided recording with silence removed.
		 */
		std::string CutTemplate(const std::string& data);

		/**
		 * \brief Runs enrollment on float samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording to be cut.
		 * \param [in]  array_length       Size of data in number of elements.
		 * \param [out]  data_out           Output buffer for the cut template.
		 * \param [out]  array_length_out   Pointer to variable being filled with
		 * 							       the size of the cut template in number of elements.
		 * \return 0 on success, non zero on error.
		 */
		int CutTemplate(const float* const data, const int array_length, float* const data_out, int* array_length_out);

		/**
		 * \brief Runs enrollment on int16_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording to be cut.
		 * \param [in]  array_length       Size of data in number of elements.
		 * \param [out]  data_out           Output buffer for the cut template.
		 * \param [out]  array_length_out   Pointer to variable being filled with
		 * 							       the size of the cut template in number of elements.
		 * \return 0 on success, non zero on error.
		 */
		int CutTemplate(const int16_t* const data, const int array_length, int16_t* const data_out, int* array_length_out);

		/**
		 * \brief Runs enrollment on int32_t samples.
		 *
		 * If NumChannels() > 1, e.g., NumChannels() == 2,
		 * then the array is as follows:
		 *
		 *  d1c1, d1c2, d2c1, d2c2, d3c1, d3c2, ..., dNc1, dNc2
		 *
		 * where d1c1 means data point 1 of channel 1.
		 *
		 * \param [in]  data               Recording to be cut.
		 * \param [in]  array_length       Size of data in number of elements.
		 * \param [out]  data_out           Output buffer for the cut template.
		 * \param [out]  array_length_out   Pointer to variable being filled with
		 * 							       the size of the cut template in number of elements.
		 * \return 0 on success, non zero on error.
		 */
		int CutTemplate(const int32_t* const data, const int array_length, int32_t* const data_out, int* array_length_out);

		/**
		 * \brief Resets the enrollment.
		 *
		 * After reset the pipeline will behave identical to a freshly constructed instance.
		 *
		 * \return true on success\n
		 * 		   false on error
		 */
		bool Reset();

		/**
		 * \brief Returns the expected sample rate for audio provided to RunDetection().
		 * \return The expected samplerate.
		 */
		int SampleRate() const;

		/**
		 * \brief Returns the expected number of channels for audio provided to RunDetection().
		 * \return The expected number of channels.
		 */
		int NumChannels() const;

		/**
		 * \brief Returns the expected number of bits for audio provided to RunDetection().
		 * \return The expected number of bits.
		 */
		int BitsPerSample() const;

		/** \brief Destructor */
		~SnowboyTemplateCut();

	private:
		std::unique_ptr<WaveHeader> wave_header_;
		std::unique_ptr<PipelineTemplateCut> cut_pipeline_;
	};

} // namespace snowboy
