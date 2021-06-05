#include <matrix-wrapper.h>
#include <nnet-component.h>
#include <ostream>
#include <snowboy-error.h>
#include <snowboy-io.h>

namespace snowboy {

	size_t ChunkInfo::GetIndex(size_t offset) const {
		if (m_offsets.empty())
		{ // if data is contiguous
			SNOWBOY_ASSERT((offset <= m_last_offset) && (offset >= m_first_offset));
			return offset - m_first_offset;
		} else
		{
			auto iter = std::lower_bound(m_offsets.begin(), m_offsets.end(), offset);
			// make sure offset is present in the vector
			SNOWBOY_ASSERT(iter != m_offsets.end() && *iter == offset);
			return static_cast<size_t>(iter - m_offsets.begin());
		}
	}

	size_t ChunkInfo::GetOffset(size_t index) const {
		if (m_offsets.empty())
		{											// if data is contiguous
			size_t offset = index + m_first_offset; // just offset by the first_offset_
			SNOWBOY_ASSERT((offset <= m_last_offset) && (offset >= m_first_offset));
			return offset;
		} else
		{
			SNOWBOY_ASSERT(index < m_offsets.size());
			return m_offsets[index];
		}
	}

	void ChunkInfo::CheckSize(const MatrixBase& mat) const {
		SNOWBOY_ASSERT(mat.rows() == NumRows() && mat.cols() == NumCols());
	}

	void ChunkInfo::Check() const {
		// Checking sanity of the ChunkInfo object
		if (!m_offsets.empty())
		{
			SNOWBOY_ASSERT((m_first_offset == m_offsets.front()) && (m_last_offset == m_offsets.back()));
		} else
		{
			SNOWBOY_ASSERT(m_last_offset >= m_first_offset);
			// asserting the chunk is not contiguous, as offsets is not empty
			SNOWBOY_ASSERT(m_last_offset - m_first_offset + 1 > m_offsets.size());
		}
		SNOWBOY_ASSERT(NumRows() % m_num_chunks == 0);
	}

	std::ostream& operator<<(std::ostream& os, const ChunkInfo& e) {
		os << "{.m_feat_dim=" << e.m_feat_dim
		   << ", .m_num_chunks=" << e.m_num_chunks
		   << ", .m_first_offset=" << e.m_first_offset
		   << ", .m_last_offset=" << e.m_last_offset
		   << ", .m_offsets={ ";
		for (auto it = e.m_offsets.begin(); it != e.m_offsets.end(); it++) {
			if (it != e.m_offsets.end()) os << ", ";
			os << *it;
		}
		os << " }}";
		return os;
	}

	int32_t Component::Index() const {
		return m_index;
	}

	void Component::SetIndex(int32_t index) {
		m_index = index;
	}

	std::vector<int32_t> Component::Context() const {
		return std::vector<int32_t>(1, 0);
	}

	bool Component::HasDataRearragement() const {
		return false;
	}

	std::unique_ptr<Component> Component::NewComponentOfType(const std::string& type) {
		if (type == "SoftmaxComponent")
			return std::unique_ptr<Component>(new SoftmaxComponent());
		if (type == "RectifiedLinearComponent")
			return std::unique_ptr<Component>(new RectifiedLinearComponent());
		if (type == "NormalizeComponent")
			return std::unique_ptr<Component>(new NormalizeComponent());
		if (type == "AffineComponent")
			return std::unique_ptr<Component>(new AffineComponent());
		if (type == "CmvnComponent")
			return std::unique_ptr<Component>(new CmvnComponent());
		if (type == "PosteriorMapComponent")
			return std::unique_ptr<Component>(new PosteriorMapComponent());
		if (type == "SpliceComponent")
			return std::unique_ptr<Component>(new SpliceComponent());
		return nullptr;
	}

	std::unique_ptr<Component> Component::ReadNew(bool binary, std::istream* is) {
		std::string token;
		ReadToken(binary, &token, is);
		if (token.size() <= 2)
			throw snowboy_exception{"Invalid component token " + token};
		// Remove leading < and following >
		token = token.substr(1, token.size() - 2);
		auto ptr = NewComponentOfType(token);
		if (ptr == nullptr)
			throw snowboy_exception{"Unknown component type " + token};
		ptr->Read(binary, is);
		return ptr;
	}

	std::string AffineComponent::Type() const {
		return "AffineComponent";
	}

	int32_t AffineComponent::InputDim() const {
		return m_linear_params.m_cols;
	}

	int32_t AffineComponent::OutputDim() const {
		return m_linear_params.m_rows;
	}

	void AffineComponent::Propagate(const ChunkInfo& in_info,
									const ChunkInfo& out_info,
									Matrix&& in,
									Matrix* out) const {
		in_info.CheckSize(in);
		out->Resize(out_info.NumChunks() * out_info.ChunkSize(), out_info.NumCols());
		out_info.CheckSize(*out);
		out->CopyRowsFromVec(m_bias_params);
		out->AddMatMat(1.0, in, MatrixTransposeType::kNoTrans, m_linear_params, MatrixTransposeType::kTrans, 1.0);
	}

	void AffineComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<LinearParams>", is);
		m_linear_params.Read(binary, is);
		ExpectToken(binary, "<BiasParams>", is);
		m_bias_params.Read(binary, is);
		m_is_gradient = 1;
		ExpectToken(binary, end_token, is);
	}

	void AffineComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<LinearParams>", os);
		m_linear_params.Write(binary, os);
		WriteToken(binary, "<BiasParams>", os);
		m_bias_params.Write(binary, os);
		WriteToken(binary, end_token, os);
	}

	Component* AffineComponent::Copy() const {
		auto res = new AffineComponent();
		res->m_is_gradient = m_is_gradient;
		res->m_linear_params = m_linear_params;
		res->m_bias_params = m_bias_params;
		return res;
	}

	std::string CmvnComponent::Type() const {
		return "CmvnComponent";
	}

	int32_t CmvnComponent::InputDim() const {
		return m_scales.size();
	}

	int32_t CmvnComponent::OutputDim() const {
		return m_scales.size();
	}

	void CmvnComponent::Propagate(const ChunkInfo& in_info,
								  const ChunkInfo& out_info,
								  Matrix&& in,
								  Matrix* out) const {
		in_info.CheckSize(in);
		*out = std::move(in);
		out_info.CheckSize(*out);
		out->MulColsVec(m_scales);
		out->AddVecToRows(1.0, m_offsets);
	}

	void CmvnComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<Scales>", is);
		m_scales.Read(binary, is);
		ExpectToken(binary, "<Offsets>", is);
		m_offsets.Read(binary, is);
		ExpectToken(binary, end_token, is);
		field_xc = 1;
	}

	void CmvnComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<Scales>", os);
		m_scales.Write(binary, os);
		WriteToken(binary, "<Offsets>", os);
		m_offsets.Write(binary, os);
		WriteToken(binary, end_token, os);
	}

	Component* CmvnComponent::Copy() const {
		auto res = new CmvnComponent();
		res->field_xc = field_xc;
		res->m_scales = m_scales;
		res->m_offsets = m_offsets;
		return res;
	}

	std::string NormalizeComponent::Type() const {
		return "NormalizeComponent";
	}

	int32_t NormalizeComponent::InputDim() const {
		return m_dim;
	}

	int32_t NormalizeComponent::OutputDim() const {
		return m_dim;
	}

	void NormalizeComponent::Propagate(const ChunkInfo& in_info,
									   const ChunkInfo& out_info,
									   Matrix&& in,
									   Matrix* out) const {
		in_info.CheckSize(in);

		Vector vec;
		vec.Resize(in.m_rows);
		vec.AddDiagMat2(1.0 / in.m_cols, in, MatrixTransposeType::kNoTrans, 0.0);
		vec.ApplyFloor(field_x14);
		vec.ApplyPow(-0.5);

		*out = std::move(in);
		out_info.CheckSize(*out);
		out->MulRowsVec(vec);
	}

	void NormalizeComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<Dim>", is);
		ReadBasicType<int32_t>(binary, &m_dim, is);
		ExpectToken(binary, end_token, is);
		field_x10 = 1;
	}

	void NormalizeComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<Dim>", os);
		WriteBasicType<int32_t>(binary, m_dim, os);
		WriteToken(binary, end_token, os);
	}

	Component* NormalizeComponent::Copy() const {
		auto res = new NormalizeComponent();
		res->m_dim = m_dim;
		res->field_x10 = field_x10;
		res->field_x14 = field_x14;
		return res;
	}

	std::string PosteriorMapComponent::Type() const {
		return "PosteriorMapComponent";
	}

	int32_t PosteriorMapComponent::InputDim() const {
		return m_inputDim;
	}

	int32_t PosteriorMapComponent::OutputDim() const {
		return m_outputDim;
	}

	void PosteriorMapComponent::Propagate(const ChunkInfo& in_info,
										  const ChunkInfo& out_info,
										  Matrix&& in,
										  Matrix* out) const {
		in_info.CheckSize(in);
		out->Resize(out_info.NumChunks() * out_info.ChunkSize(), out_info.NumCols());
		out_info.CheckSize(*out);

		for (size_t r = 0; r < in.m_rows; r++)
		{
			if (out->m_cols < 2)
			{
				out->m_data[out->m_stride * r] = 1.0;
			} else {
				// TODO: I did my best but between here
				auto ptr = out->m_data + (out->m_stride * r);
				float sum = 0.0f;
				for (auto& idx_vec : m_indices) {
					ptr++;
					for (auto idx : idx_vec) {
						auto v = in.m_data[in.m_stride * r + idx];
						*ptr += v;
						sum += v;
					}
				}
				out->m_data[out->m_stride * r] = 1.0 - sum;
				// TODO: and here are probably a number of bugs
			}
		}
	}

	void PosteriorMapComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<InputDim>", is);
		ReadBasicType<int32_t>(binary, &m_inputDim, is);
		ExpectToken(binary, "<OutputDim>", is);
		ReadBasicType<int32_t>(binary, &m_outputDim, is);
		ExpectToken(binary, "<Indices>", is);
		m_indices.resize(m_outputDim - 1);
		for (auto& e : m_indices)
			ReadIntegerVector<int32_t>(binary, &e, is);
		ExpectToken(binary, end_token, is);
		field_xc = 1;
	}

	void PosteriorMapComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<InputDim>", os);
		WriteBasicType<int32_t>(binary, m_inputDim, os);
		WriteToken(binary, "<OutputDim>", os);
		WriteBasicType<int32_t>(binary, m_outputDim, os);
		WriteToken(binary, "<Indices>", os);
		for (auto& e : m_indices)
			WriteIntegerVector(binary, e, os);
		WriteToken(binary, end_token, os);
	}

	Component* PosteriorMapComponent::Copy() const {
		auto res = new PosteriorMapComponent();
		res->field_xc = field_xc;
		res->m_inputDim = m_inputDim;
		res->m_outputDim = m_outputDim;
		res->m_indices = m_indices;
		return res;
	}

	std::string RectifiedLinearComponent::Type() const {
		return "RectifiedLinearComponent";
	}

	int32_t RectifiedLinearComponent::InputDim() const {
		return m_dim;
	}

	int32_t RectifiedLinearComponent::OutputDim() const {
		return m_dim;
	}

	void RectifiedLinearComponent::Propagate(const ChunkInfo& in_info,
											 const ChunkInfo& out_info,
											 Matrix&& in,
											 Matrix* out) const {
		in_info.CheckSize(in);
		*out = std::move(in);
		out_info.CheckSize(*out);
		out->ApplyFloor(0.0);
	}

	void RectifiedLinearComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<Dim>", is);
		ReadBasicType<int32_t>(binary, &m_dim, is);
		ExpectToken(binary, end_token, is);
		field_x10 = 1;
	}

	void RectifiedLinearComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<Dim>", os);
		WriteBasicType<int32_t>(binary, m_dim, os);
		WriteToken(binary, end_token, os);
	}

	Component* RectifiedLinearComponent::Copy() const {
		auto res = new RectifiedLinearComponent();
		res->m_dim = m_dim;
		res->field_x10 = field_x10;
		return res;
	}

	std::string SoftmaxComponent::Type() const {
		return "SoftmaxComponent";
	}

	int32_t SoftmaxComponent::InputDim() const {
		return m_dim;
	}

	int32_t SoftmaxComponent::OutputDim() const {
		return m_dim;
	}

	void SoftmaxComponent::Propagate(const ChunkInfo& in_info,
									 const ChunkInfo& out_info,
									 Matrix&& in,
									 Matrix* out) const {
		in_info.CheckSize(in);

		*out = std::move(in);
		out_info.CheckSize(*out);
		for (size_t i = 0; i < out->rows(); i++)
		{
			SubVector vec{*out, i};
			vec.ApplySoftmax();
		}
		// This floor on the output helps us deal with
		// almost-zeros in a way that doesn't lead to overflow.
		out->ApplyFloor(1.0e-20);
	}

	void SoftmaxComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<Dim>", is);
		ReadBasicType<int32_t>(binary, &m_dim, is);
		ExpectToken(binary, end_token, is);
		field_x10 = 1;
	}

	void SoftmaxComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<Dim>", os);
		WriteBasicType<int32_t>(binary, m_dim, os);
		WriteToken(binary, end_token, os);
	}

	Component* SoftmaxComponent::Copy() const {
		auto res = new SoftmaxComponent();
		res->m_dim = m_dim;
		res->field_x10 = field_x10;
		return res;
	}

	std::string SpliceComponent::Type() const {
		return "SpliceComponent";
	}

	int32_t SpliceComponent::InputDim() const {
		return m_inputDim;
	}

	int32_t SpliceComponent::OutputDim() const {
		return m_context.size() * (m_inputDim - m_constComponentDim) + m_constComponentDim;
	}

	std::vector<int32_t> SpliceComponent::Context() const {
		return m_context;
	}

	bool SpliceComponent::HasDataRearragement() const {
		return true;
	}

	void SpliceComponent::Propagate(const ChunkInfo& in_info,
									const ChunkInfo& out_info,
									Matrix&& in,
									Matrix* out) const {
		in_info.CheckSize(in);
		out->Resize(out_info.NumChunks() * out_info.ChunkSize(), out_info.NumCols());
		out_info.CheckSize(*out);
		SNOWBOY_ASSERT(in_info.NumChunks() == out_info.NumChunks());

		auto in_chunk_size = in_info.ChunkSize();
		auto out_chunk_size = out_info.ChunkSize();
		auto input_dim = in_info.NumCols();

		if (out_chunk_size <= 0)
			throw snowboy_exception{"Zero output dimension in SpliceComponent"};

		auto num_splice = m_context.size();
		std::vector<std::vector<ssize_t>> indexes(num_splice);
		for (auto& e : indexes)
			e.resize(out->m_rows);

		auto const_dim = m_constComponentDim;
		std::vector<ssize_t> const_indexes;
		const_indexes.resize((const_dim == 0) ? 0u : out->m_rows);

		for (size_t chunk = 0; chunk < in_info.NumChunks(); chunk++)
		{
			if (chunk == 0)
			{
				// this branch could be used for all chunks in the matrix,
				// but is restricted to chunk 0 for efficiency reasons
				for (size_t c = 0; c < num_splice; c++)
				{
					for (size_t out_index = 0; out_index < out_chunk_size; out_index++)
					{
						int32_t out_offset = out_info.GetOffset(out_index);
						int32_t in_index = in_info.GetIndex(out_offset + m_context[c]);
						indexes[c][chunk * out_chunk_size + out_index] = chunk * in_chunk_size + in_index;
					}
				}
			} else
			{ // just copy the indices from the previous chunk
				// and offset these by input chunk size
				for (size_t c = 0; c < num_splice; c++)
				{
					for (size_t out_index = 0; out_index < out_chunk_size; out_index++)
					{
						int32_t last_value = indexes[c][(chunk - 1) * out_chunk_size + out_index];
						indexes[c][chunk * out_chunk_size + out_index] = (last_value == -1 ? -1 : last_value + in_chunk_size);
					}
				}
			}
			if (const_dim != 0)
			{
				for (size_t out_index = 0; out_index < out_chunk_size; out_index++)
					const_indexes[chunk * out_chunk_size + out_index] = chunk * in_chunk_size + out_index; // there is
																										   // an arbitrariness here; since we assume the const_component
																										   // is constant within a chunk, it doesn't matter from where we copy.
			}
		}

		for (size_t c = 0; c < num_splice; c++)
		{
			size_t dim = input_dim - const_dim; // dimension we
			// are splicing
			SubMatrix in_part(in, 0, in.m_rows, 0, dim);
			SubMatrix out_part(*out, 0, out->m_rows, c * dim, dim);
			out_part.CopyRows(in_part, indexes[c]);
		}
		if (const_dim != 0)
		{
			SubMatrix in_part(in, 0, in.m_rows, in.m_cols - const_dim, const_dim);
			SubMatrix out_part(*out, 0, out->m_rows, out->m_cols - const_dim, const_dim);
			out_part.CopyRows(in_part, const_indexes);
		}
	}

	void SpliceComponent::Read(bool binary, std::istream* is) {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		ExpectOneOrTwoTokens(binary, beg_token, "<InputDim>", is);
		ReadBasicType<int32_t>(binary, &m_inputDim, is);
		ExpectToken(binary, "<Context>", is);
		ReadIntegerVector<int>(binary, &m_context, is);
		ExpectToken(binary, "<ConstComponentDim>", is);
		ReadBasicType<int32_t>(binary, &m_constComponentDim, is);
		ExpectToken(binary, end_token, is);
		field_xc = 1;
	}

	void SpliceComponent::Write(bool binary, std::ostream* os) const {
		auto beg_token = "<" + Type() + ">";
		auto end_token = "</" + Type() + ">";
		WriteToken(binary, beg_token, os);
		WriteToken(binary, "<InputDim>", os);
		WriteBasicType<int32_t>(binary, m_inputDim, os);
		WriteToken(binary, "<Context>", os);
		WriteIntegerVector<int32_t>(binary, m_context, os);
		WriteToken(binary, "<ConstComponentDim>", os);
		WriteBasicType<int32_t>(binary, m_constComponentDim, os);
		WriteToken(binary, end_token, os);
	}

	Component* SpliceComponent::Copy() const {
		auto res = new SpliceComponent();
		res->field_xc = field_xc;
		res->m_inputDim = m_inputDim;
		res->m_constComponentDim = m_constComponentDim;
		res->m_context = m_context;
		return res;
	}

} // namespace snowboy
