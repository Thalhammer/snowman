#include <cassert>
#include <frame-info.h>
#include <nnet-component.h>
#include <nnet-lib.h>
#include <set>
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
		m_components.resize(other.m_components.size());
		for (size_t i = 0; i < m_components.size(); i++)
			m_components[i].reset(other.m_components[i]->Copy());
	}

	Nnet::~Nnet() {
		Destroy();
	}

	void Nnet::Compute(const MatrixBase& input, const std::vector<FrameInfo>& b, Matrix* output, std::vector<FrameInfo>* d) {
		if (input.m_rows == 0) {
			output->Resize(0, 0);
			d->clear();
			return;
		}
		if (m_is_first_chunk == 0) {
			m_input_data.Resize(input.m_rows + m_unprocessed_buffer.m_rows, input.m_cols);
			if (m_unprocessed_buffer.m_rows > 0) {
				m_input_data.RowRange(0, m_unprocessed_buffer.m_rows).CopyFromMat(m_unprocessed_buffer, MatrixTransposeType::kNoTrans);
			}
			m_input_data.RowRange(m_unprocessed_buffer.m_rows, input.m_rows).CopyFromMat(input, MatrixTransposeType::kNoTrans);
			m_unprocessed_buffer.Resize(0, 0);
		} else {
			m_is_first_chunk = 0;
			if (m_pad_input) {
				if (m_left_context > 0) {
					m_input_data.Resize(input.m_rows + m_left_context, input.m_cols);
					m_input_data.RowRange(0, m_left_context).CopyRowsFromVec(SubVector{input, 0});
					m_input_data.RowRange(m_left_context, input.m_rows).CopyFromMat(input, MatrixTransposeType::kNoTrans);
				} else {
					m_input_data.Resize(input.m_rows, input.m_cols);
					m_input_data.CopyFromMat(input, MatrixTransposeType::kNoTrans);
				}
			} else {
				m_input_data.Resize(input.m_rows, input.m_cols);
				m_input_data.CopyFromMat(input, MatrixTransposeType::kNoTrans);
			}
		}
		auto num_effective_input_rows = field_xa ? (m_input_data.m_rows + LeftContext() + RightContext()) : m_input_data.m_rows;
		if (num_effective_input_rows > m_left_context + m_right_context) {
			if (field_x18 != num_effective_input_rows) {
				ComputeChunkInfo(num_effective_input_rows, 1);
				field_x18 = num_effective_input_rows;
			}
			field_b8 = SubVector{m_input_data, m_input_data.rows() - 1};
			Propagate();
			*output = m_output_data;
			m_output_data.Resize(0, 0);
		} else {
			m_unprocessed_buffer = m_input_data;
			field_b8 = SubVector{m_input_data, m_input_data.rows() - 1};
			m_input_data.Resize(0, 0);
			output->Resize(0, 0);
		}
		for (auto& frame : b) {
			field_x20.push_back(frame);
		}
		if (field_xc == 0 && m_pad_input == 0 && input.m_rows > 0) {
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
		const size_t output_chunk_size = (input_chunk_size - m_left_context) - m_right_context;
		SNOWBOY_ASSERT(output_chunk_size > 0);
		std::vector<size_t> current_output_inds;
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
		m_components.clear();
	}

	void Nnet::FlushOutput(const MatrixBase& param_1, const std::vector<FrameInfo>& param_2, Matrix* param_3, std::vector<FrameInfo>* param_4) {
		param_3->Resize(0, 0);
		param_4->clear();
		if (param_1.m_rows > 0)
			Compute(param_1, param_2, param_3, param_4);

		auto uVar10 = m_unprocessed_buffer.m_rows;
		auto num_effective_input_rows_new = (field_xa ? LeftContext() + RightContext() : 0) + m_unprocessed_buffer.m_rows;

		if (m_pad_input && field_b8.size() > 0) {
			auto t = RightContext();
			num_effective_input_rows_new += t;
			uVar10 += t;
		}

		if (LeftContext() + RightContext() < num_effective_input_rows_new) {
			m_input_data.Resize(uVar10, InputDim());
			if (m_unprocessed_buffer.m_rows > 0) {
				m_input_data.RowRange(0, m_unprocessed_buffer.m_rows).CopyFromMat(m_unprocessed_buffer, MatrixTransposeType::kNoTrans);
			}
			assert(m_right_context == RightContext());
			if (m_pad_input && 0 < RightContext()) {
				m_input_data.RowRange(m_unprocessed_buffer.m_rows, RightContext()).CopyRowsFromVec(field_b8);
			}
			if (num_effective_input_rows_new != field_x18) {
				ComputeChunkInfo(num_effective_input_rows_new, 1);
				field_x18 = num_effective_input_rows_new;
			}
			Propagate();
			if (m_output_data.m_rows > 0) {
				if (param_3->m_rows != 0) {
					param_3->Resize(m_output_data.m_rows + param_3->m_rows, param_3->m_cols, MatrixResizeType::kCopyData);
					param_3->RowRange(param_3->m_rows - m_output_data.m_rows, m_output_data.m_rows).CopyFromMat(m_output_data, MatrixTransposeType::kNoTrans);
				} else {
					*param_3 = m_output_data;
				}
			}
			m_output_data.Resize(0, 0);
		}

		param_4->resize(param_3->m_rows);
		for (auto uVar7 = param_4->size() - field_x20.size(); uVar7 < param_4->size(); uVar7++) {
			param_4->at(uVar7) = field_x20.front();
			field_x20.pop_front();
		}
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
		for (size_t c = 0; c < m_components.size(); c++) {
			auto ctx = m_components[c]->Context();
			auto inputDim = m_components[c]->InputDim();
			if (ctx.size() > 1) {
				auto& rci = m_reusable_component_inputs[c];
				if (rci.m_rows > 0) {
					Matrix local_98;
					local_98.Resize(rci.m_rows + m_input_data.m_rows, inputDim);
					local_98.RowRange(0, rci.m_rows).CopyFromMat(rci, MatrixTransposeType::kNoTrans);
					local_98.RowRange(rci.m_rows, m_input_data.m_rows).CopyFromMat(m_input_data, MatrixTransposeType::kNoTrans);
					m_input_data = std::move(local_98);
				}
				rci.Resize(ctx.back() - ctx.front(), inputDim);
				rci.CopyFromMat(m_input_data.RowRange(m_input_data.m_rows - rci.m_rows, rci.m_rows), MatrixTransposeType::kNoTrans);
			}
			m_chunkinfo[c].MakeOffsetsContiguous();
			m_chunkinfo[c + 1].MakeOffsetsContiguous();
			auto last_offset = m_chunkinfo[c].GetOffset(m_chunkinfo[c].ChunkSize() - 1);
			ChunkInfo input_chunk_info{
				m_chunkinfo[c].NumCols(),
				m_chunkinfo[c].NumChunks(),
				last_offset - m_input_data.rows() + 1,
				last_offset};
			last_offset = m_chunkinfo[c + 1].GetOffset(m_chunkinfo[c + 1].ChunkSize() - 1);
			ChunkInfo output_chunk_info{
				m_chunkinfo[c + 1].NumCols(),
				m_chunkinfo[c + 1].NumChunks(),
				last_offset - (m_input_data.rows() - (ctx.back() - ctx.front())) + 1,
				last_offset};
			m_components[c]->Propagate(input_chunk_info, output_chunk_info, std::move(m_input_data), &m_output_data);
			if (c < m_components.size() - 1) {
				m_input_data = std::move(m_output_data);
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
		for (size_t i = 0; i < m_components.size(); i++) {
			m_components[i]->SetIndex(i);
		}
	}

	void Nnet::Read(bool binary, std::istream* is) {
		Destroy();
		ExpectToken(binary, "<Nnet>", is);
		ExpectToken(binary, "<NumComponents>", is);
		int num_components;
		ReadBasicType<int32_t>(binary, &num_components, is);
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
			for (auto& e : m_components) {
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
		WriteBasicType<int32_t>(binary, m_components.size(), os);
		WriteToken(binary, "<Components>", os);
		for (auto& e : m_components) {
			e->Write(binary, os);
		}
		WriteToken(binary, "</Components>", os);
		WriteToken(binary, "</Nnet>", os);
	}

	int32_t Nnet::LeftContext() const {
		int32_t ctx = 0;
		for (size_t i = 0; i < m_components.size(); i++) {
			ctx += m_components[i]->Context().front();
		}
		return -ctx;
	}

	int32_t Nnet::RightContext() const {
		int32_t ctx = 0;
		for (size_t i = 0; i < m_components.size(); i++) {
			ctx += m_components[i]->Context().back();
		}
		return ctx;
	}

} // namespace snowboy
