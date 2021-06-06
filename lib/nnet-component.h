#pragma once
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <matrix-wrapper.h>
#include <memory>
#include <vector-wrapper.h>
#include <vector>

namespace snowboy {
	struct MatrixBase;
	class ChunkInfo {
		size_t m_feat_dim;
		size_t m_num_chunks;
		size_t m_first_offset;
		size_t m_last_offset;
		std::vector<size_t> m_offsets;
		friend std::ostream& operator<<(std::ostream& os, const ChunkInfo& e);

	public:
		ChunkInfo() noexcept // default constructor we assume this object will not be used
			: m_feat_dim(0), m_num_chunks(0),
			  m_first_offset(0), m_last_offset(0),
			  m_offsets() {}

		ChunkInfo(size_t feat_dim, size_t num_chunks,
				  size_t first_offset, size_t last_offset) noexcept
			: m_feat_dim(feat_dim), m_num_chunks(num_chunks),
			  m_first_offset(first_offset), m_last_offset(last_offset),
			  m_offsets() { Check(); }

		ChunkInfo(size_t feat_dim, size_t num_chunks,
				  const std::vector<size_t> offsets)
			: m_feat_dim(feat_dim), m_num_chunks(num_chunks),
			  m_first_offset(offsets.front()), m_last_offset(offsets.back()),
			  m_offsets(offsets) {
			if (m_last_offset - m_first_offset + 1 == m_offsets.size())
				m_offsets.clear();
			Check();
		}

		size_t GetIndex(size_t offset) const;
		size_t GetOffset(size_t index) const;

		void CheckSize(const MatrixBase&) const;
		void Check() const;

		// Not in snowboy
		size_t NumRows() const { return m_num_chunks * ChunkSize(); }
		size_t NumCols() const { return m_feat_dim; }
		size_t NumChunks() const { return m_num_chunks; }
		size_t ChunkSize() const { return !m_offsets.empty() ? m_offsets.size() : (m_last_offset - m_first_offset + 1); }
		void MakeOffsetsContiguous() {
			m_offsets.clear();
			Check();
		}
	};
	std::ostream& operator<<(std::ostream& os, const ChunkInfo& e);

	class Component {
	public:
		Component() : m_index(-1) {}

		virtual std::string Type() const = 0;
		virtual int32_t Index() const;
		virtual void SetIndex(int32_t index);
		virtual int32_t InputDim() const = 0;
		virtual int32_t OutputDim() const = 0;
		virtual std::vector<int32_t> Context() const;
		virtual bool HasDataRearragement() const;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const = 0;

		virtual void Read(bool binary, std::istream* is) = 0;
		virtual void Write(bool binary, std::ostream* os) const = 0;
		virtual Component* Copy() const = 0;
		virtual ~Component() {}

		static std::unique_ptr<Component> NewComponentOfType(const std::string& type);
		static std::unique_ptr<Component> ReadNew(bool binary, std::istream* is);

	private:
		int32_t m_index;
		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;
		Component(Component&&) = delete;
		Component& operator=(Component&&) = delete;
	};

	class AffineComponent : public Component {
		bool m_is_gradient = 1;
		Matrix m_linear_params;
		Vector m_bias_params;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~AffineComponent() {}
	};

	class CmvnComponent : public Component {
		bool field_xc = 0;
		Vector m_scales;
		Vector m_offsets;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~CmvnComponent() {}
	};

	class NormalizeComponent : public Component {
		int32_t m_dim = 0;
		bool field_x10 = 0;
		float field_x14 = pow(2.0, -66);

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~NormalizeComponent() {}
	};

	class PosteriorMapComponent : public Component {
		bool field_xc;
		int32_t m_inputDim;
		int32_t m_outputDim;
		std::vector<std::vector<int>> m_indices;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~PosteriorMapComponent() {}
	};

	class RectifiedLinearComponent : public Component {
		int32_t m_dim;
		bool field_x10;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~RectifiedLinearComponent() {}
	};

	class SoftmaxComponent : public Component {
		int32_t m_dim;
		bool field_x10;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~SoftmaxComponent() {}
	};

	class SpliceComponent : public Component {
		bool field_xc;
		int m_inputDim;
		int m_constComponentDim;
		std::vector<int> m_context;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual std::vector<int32_t> Context() const override;
		virtual bool HasDataRearragement() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   Matrix&& in,
							   Matrix* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~SpliceComponent() {}
	};
} // namespace snowboy
