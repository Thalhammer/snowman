#include <frame-info.h>
#include <iostream>
#include <nnet-component.h>
#include <nnet-lib.h>
#include <set>
#include <snowboy-debug.h>
#include <snowboy-io.h>

namespace snowboy {
	Nnet::Nnet() {
		m_pad_input = 1;
		m_is_first_chunk = 1;
		field_xa = 0;
		field_xb = 0;
		field_xc = 0;
		m_left_context = 0;
		m_right_context = 0;
		field_x18 = 0;
	}

	Nnet::Nnet(bool pad_context) {
		m_pad_input = pad_context;
		m_is_first_chunk = 1;
		field_xa = 0;
		field_xb = 0;
		field_xc = 0;
		m_left_context = 0;
		m_right_context = 0;
		field_x18 = 0;
	}

	Nnet::Nnet(const Nnet& other) {
		m_pad_input = other.m_pad_input;
		m_is_first_chunk = other.m_is_first_chunk;
		field_xa = other.field_xa;
		field_xb = other.field_xb;
		field_xc = other.field_xc;
		m_left_context = other.m_left_context;
		m_right_context = other.m_right_context;
		field_x18 = 0;
		field_x20 = other.field_x20;
		m_chunkinfo = other.m_chunkinfo;
		m_reusable_component_inputs = other.m_reusable_component_inputs;
		field_b8 = other.field_b8;
		m_unprocessed_buffer = other.m_unprocessed_buffer;
		m_input_data = other.m_input_data;
		m_output_data = other.m_output_data;
		m_components = other.m_components;
		for (auto& e : m_components)
			e = e->Copy();
	}

	Nnet::~Nnet() {
		Destroy();
	}

	void Nnet::Compute(const MatrixBase& input, const std::vector<FrameInfo>& b, Matrix* output, std::vector<FrameInfo>* d) {
		//std::cout << "Nnet::Compute(" << input << ", b.size()=" << b.size() << ", output=" << *output << ", d.size()=" << d->size() << ")" << std::endl;
		if (input.m_rows == 0) {
			output->Resize(0, 0);
			d->clear();
			return;
		}
		int32_t num_effective_input_rows = 0;
		if (m_is_first_chunk == 0) {
			m_input_data.Resize(input.m_rows + m_unprocessed_buffer.m_rows, input.m_cols);
			if (m_unprocessed_buffer.m_rows > 0) {
				m_input_data.RowRange(0, m_unprocessed_buffer.m_rows).CopyFromMat(m_unprocessed_buffer, MatrixTransposeType::kNoTrans);
			}
			m_input_data.RowRange(m_unprocessed_buffer.m_rows, input.m_rows).CopyFromMat(input, MatrixTransposeType::kNoTrans);
			m_unprocessed_buffer.Resize(0, 0);
			num_effective_input_rows = m_left_context + m_right_context;
		} else {
			m_is_first_chunk = 0;
			if (m_pad_input && m_left_context > 0) {
				m_input_data.Resize(input.m_rows + m_left_context, input.m_cols);
				m_input_data.RowRange(0, m_left_context).CopyRowsFromVec(SubVector{input, 0});
				m_input_data.RowRange(m_left_context, input.m_rows).CopyFromMat(input, MatrixTransposeType::kNoTrans);
			} else {
				m_input_data.Resize(input.m_rows, input.m_cols);
				m_input_data.CopyFromMat(input, MatrixTransposeType::kNoTrans);
			}
			num_effective_input_rows = m_input_data.m_rows;
		}
		if (num_effective_input_rows >= m_left_context + m_right_context + 1) {
			if (field_x18 != num_effective_input_rows) {
				ComputeChunkInfo(num_effective_input_rows, 1);
				field_x18 = num_effective_input_rows;
			}
			field_b8 = SubVector{m_input_data, static_cast<int32_t>(m_input_data.m_rows) - 1};
			Propagate();
			*output = m_output_data;
		} else {
			m_unprocessed_buffer = m_input_data;
			field_b8 = SubVector{m_input_data, static_cast<int32_t>(m_input_data.m_rows) - 1};
			m_input_data.Resize(0, 0);
			output->Resize(0, 0);
		}
		for (auto& frame : b) {
			field_x20.push_back(frame);
		}
		if (field_xc == 0 && m_pad_input == 0) {
			for (int i = 0; i < m_left_context; i++) {
				field_x20.pop_front();
			}
			field_xc = 1;
		}
		d->resize(output->m_rows);
		for (auto& e : *d) {
			e = field_x20.front();
			field_x20.pop_front();
		}
	}

	// Note: Adopted from kaldi
	void Nnet::ComputeChunkInfo(int input_chunk_size, int num_chunks) {
		const auto output_chunk_size = (input_chunk_size - m_left_context) - m_right_context;
		SNOWBOY_ASSERT(output_chunk_size > 0);
		std::vector<int> current_output_inds;
		current_output_inds.resize(output_chunk_size);
		for (size_t i = 0; i < output_chunk_size; i++)
			current_output_inds[i] = i + m_left_context;

		// indexes for last component is empty, since the last component's chunk is
		// always contiguous
		// component's output is always contiguous
		m_chunkinfo[m_components.size()] = ChunkInfo(
			m_components[m_components.size() - 1]->OutputDim(),
			num_chunks, current_output_inds.front(),
			current_output_inds.back());

		std::vector<int32_t> current_input_inds;
		for (int32_t i = m_components.size() - 1; i >= 0; i--) {
			std::vector<int32_t> current_context = m_components[i]->Context();
			std::set<int32_t> current_input_ind_set;
			for (size_t j = 0; j < current_context.size(); j++)
				for (size_t k = 0; k < current_output_inds.size(); k++)
					current_input_ind_set.insert(current_context[j] + current_output_inds[k]);
			current_output_inds.resize(current_input_ind_set.size());
			std::copy(current_input_ind_set.begin(),
					  current_input_ind_set.end(),
					  current_output_inds.begin());

			// checking if the vector has contiguous data
			// assign indexes only if the data is not contiguous
			if (current_output_inds.size() != current_output_inds.back() - current_output_inds.front() + 1) {
				m_chunkinfo[i] = ChunkInfo(m_components[i]->InputDim(),
										   num_chunks,
										   current_output_inds);
			} else {
				m_chunkinfo[i] = ChunkInfo(m_components[i]->InputDim(),
										   num_chunks,
										   current_output_inds.front(),
										   current_output_inds.back());
			}
		}

		for (size_t i = 0; i < m_components.size(); i++) {
			m_chunkinfo[i].MakeOffsetsContiguous();
			if (m_components[i]->HasDataRearragement())
				break;
		}

		// sanity testing for chunk_info_out vector
		for (auto& e : m_chunkinfo) {
			e.Check();
		}
	}

	void Nnet::Destroy() {
		for (auto e : m_components)
			delete e;
		m_components.clear();
	}

	void Nnet::FlushOutput(const MatrixBase& a, const std::vector<FrameInfo>& b, Matrix* c, std::vector<FrameInfo>* d) {
		//std::cout << "Nnet::FlushOutput()" << std::endl;
		c->Resize(0, 0);
		d->clear();
		if (a.m_rows > 0)
			Compute(a, b, c, d);

		const auto num_stored_frames = m_left_context + m_right_context;
		const auto num_frames_padding = field_xa ? m_unprocessed_buffer.m_rows : 0;
		auto num_effective_input_rows = num_stored_frames + num_frames_padding;

		if (m_pad_input && field_b8.m_size > 0) {
			num_effective_input_rows += m_unprocessed_buffer.m_rows;
		}

		if (num_effective_input_rows > num_stored_frames) {
			auto dim = InputDim();
			m_input_data.Resize(m_unprocessed_buffer.m_rows, dim);
			if (m_unprocessed_buffer.m_rows > 0) {
				m_input_data.RowRange(0, m_unprocessed_buffer.m_rows).CopyFromMat(m_unprocessed_buffer, MatrixTransposeType::kNoTrans);
			}
			if (m_pad_input && m_right_context > 0) {
				m_input_data.RowRange(m_unprocessed_buffer.m_rows, m_right_context).CopyRowsFromVec(field_b8);
			}
			if (num_effective_input_rows != field_x18) {
				ComputeChunkInfo(num_effective_input_rows, 1);
				field_x18 = num_effective_input_rows;
			}
			Propagate();
			if (m_output_data.m_rows > 0) {
				if (c->m_rows != 0) {
					c->Resize(m_output_data.m_rows + c->m_rows, c->m_cols, MatrixResizeType::kCopyData);
					c->RowRange(c->m_rows - m_output_data.m_rows, m_output_data.m_rows).CopyFromMat(m_output_data, MatrixTransposeType::kNoTrans);
					m_output_data.Resize(0, 0);
				}
			} else {
				*c = m_output_data;
				m_output_data.Resize(0, 0);
			}
		}
		d->resize(c->m_rows);
		// TODO: No clue if this is correct
		field_x20.resize(c->m_rows);
		ResetComputation();
	}

	int32_t Nnet::InputDim() const {
		if (m_components.empty()) return 0;
		return m_components.front()->InputDim();
	}

	int32_t Nnet::OutputDim() const {
		if (m_components.empty()) return 0;
		return m_components.back()->OutputDim();
	}

	void Nnet::Propagate() {
		//std::cout << "Nnet::Propagate()" << std::endl;
		for (int32_t c = 0; c < m_components.size(); c++) {
			m_chunkinfo[c].MakeOffsetsContiguous();		// Might not be needed
			m_chunkinfo[c + 1].MakeOffsetsContiguous(); // Might not be needed

			auto component = m_components[c];
			auto context = component->Context();
			if (context.size() > 1) {
				auto dim = component->InputDim();
				auto& rci = m_reusable_component_inputs[c];
				if (rci.m_rows > 0) {
					Matrix temp;
					temp.Resize(m_input_data.m_rows + rci.m_rows, dim);
					temp.RowRange(0, rci.m_rows).CopyFromMat(rci, MatrixTransposeType::kNoTrans);
					temp.RowRange(rci.m_rows, m_input_data.m_rows).CopyFromMat(m_input_data, MatrixTransposeType::kNoTrans);
					m_input_data = temp;
				}
				rci.Resize(context.back() - context.front(), dim);
				rci.CopyFromMat(m_input_data.RowRange(m_input_data.m_rows - rci.m_rows, rci.m_rows), MatrixTransposeType::kNoTrans);
			}
			auto last_offset = m_chunkinfo[c].GetOffset(m_chunkinfo[c].ChunkSize() - 1);
			ChunkInfo input_chunk_info{
				m_chunkinfo[c].NumCols(),
				m_chunkinfo[c].NumChunks(),
				last_offset - (static_cast<int32_t>(m_input_data.m_rows)) + 1,
				last_offset};
			last_offset = m_chunkinfo[c + 1].GetOffset(m_chunkinfo[c + 1].ChunkSize() - 1);
			ChunkInfo output_chunk_info{
				m_chunkinfo[c + 1].NumCols(),
				m_chunkinfo[c + 1].NumChunks(),
				last_offset - static_cast<int32_t>(m_input_data.m_rows - (context.back() - context.front())) + 1,
				last_offset};
			m_output_data.Resize(output_chunk_info.NumRows(), output_chunk_info.NumCols());
			component->Propagate(input_chunk_info, output_chunk_info, m_input_data, &m_output_data);
			if (c < m_components.size() - 1) {
				m_input_data = m_output_data;
				m_output_data.Resize(0, 0);
			} else {
				m_input_data.Resize(0, 0);
			}
		}
		if (field_xa == 0) field_xa = 1;
	}

	void Nnet::ResetComputation() {
		m_is_first_chunk = 1;
		field_xa = 0;
		field_xc = 0;
		for (auto& e : m_reusable_component_inputs) {
			e.Resize(0, 0);
		}
		field_b8.Resize(0);
		m_unprocessed_buffer.Resize(0, 0);
		m_input_data.Resize(0, 0);
		m_output_data.Resize(0, 0);
		field_x20.clear();
		field_x18 = 0;
	}

	void Nnet::SetIndices() {
		for (int32_t i = 0; i < m_components.size(); i++) {
			m_components[i]->SetIndex(i);
		}
	}

	void Nnet::Read(bool binary, std::istream* is) {
		Destroy();
		ExpectToken(binary, "<Nnet>", is);
		ExpectToken(binary, "<NumComponents>", is);
		int num_components;
		ReadBasicType<int>(binary, &num_components, is);
		m_components.resize(num_components);
		ExpectToken(binary, "<Components>", is);
		for (int i = 0; i < num_components; i++) {
			m_components[i] = Component::ReadNew(binary, is);
		}
		ExpectToken(binary, "</Components>", is);
		ExpectToken(binary, "</Nnet>", is);
		SetIndices();
		m_left_context = 0;
		m_right_context = 0;
		if (!m_components.empty()) {
			// Note: This used to be two loops, one summing m_left_context and one summing m_right_context
			// Since neither have crossreferences I collapsed them into one.
			for (auto e : m_components) {
				auto ctx = e->Context();
				m_left_context += ctx.front();
				m_right_context += ctx.back();
			}
			m_left_context = -m_left_context;
		}
		field_xb = 1;
		m_chunkinfo.resize(num_components + 1);
		m_reusable_component_inputs.resize(num_components + 1);
	}

	void Nnet::Write(bool binary, std::ostream* os) const {
		WriteToken(binary, "<Nnet>", os);
		WriteToken(binary, "<NumComponents>", os);
		WriteBasicType<int>(binary, m_components.size(), os);
		WriteToken(binary, "<Components>", os);
		for (auto e : m_components) {
			e->Write(binary, os);
		}
		WriteToken(binary, "</Components>", os);
		WriteToken(binary, "</Nnet>", os);
	}

} // namespace snowboy