#pragma once
#include <cstdint>
#include <deque>
#include <iosfwd>
#include <matrix-wrapper.h>
#include <vector-wrapper.h>
#include <vector>

namespace snowboy {
	struct FrameInfo;
	struct ChunkInfo;
	struct Component;
	class Nnet {
		// TODO: Figure out names for remaining data fields...
		bool m_pad_input;
		bool m_is_first_chunk;
		bool field_xa;
		bool field_xb;
		bool field_xc;
		// Padding ?
		int m_left_context;
		int m_right_context;
		int field_x18;
		// Padding ?
		std::deque<FrameInfo> field_x20;
		std::vector<ChunkInfo> m_chunkinfo;
		std::vector<Component*> m_components;
		std::vector<Matrix> m_reusable_component_inputs;
		Vector field_b8;
		Matrix m_unprocessed_buffer;
		Matrix m_input_data;
		Matrix m_output_data;

	public:
		// This is the only virtual function and I dont think it needs to be virtual,
		// but it is virtual in the original, so we need it to get matching layout.
		virtual ~Nnet();

		Nnet();
		Nnet(bool pad_context);
		Nnet(const Nnet& other);

		void Compute(const MatrixBase&, const std::vector<FrameInfo>&, Matrix*, std::vector<FrameInfo>*);
		void ComputeChunkInfo(int, int);
		void Destroy();
		void FlushOutput(const MatrixBase&, const std::vector<FrameInfo>&, Matrix*, std::vector<FrameInfo>*);
		int32_t InputDim() const;
		int32_t OutputDim() const;
		void Propagate();
		void ResetComputation();
		void SetIndices();
		void Read(bool binary, std::istream* is);
		void Write(bool binary, std::ostream* is) const;

		int32_t LeftContext() const;
		int32_t RightContext() const;
	};
	static_assert(sizeof(Nnet) == 0x110);
} // namespace snowboy