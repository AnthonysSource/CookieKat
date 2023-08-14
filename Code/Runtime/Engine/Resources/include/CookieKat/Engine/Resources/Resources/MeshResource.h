#pragma once

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Systems/Resources/IResource.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

//-----------------------------------------------------------------------------

namespace CKE {
	class MeshLoader;
}

//-----------------------------------------------------------------------------

namespace CKE {
	struct Vertex3P2Tc
	{
		Vec3 m_Position;
		Vec2 m_TexCoord;

		Vertex3P2Tc() = default;
		Vertex3P2Tc(f32 x, f32 y, f32 z, f32 s, f32 t) : m_Position(x, y, z), m_TexCoord(s, t) {}
	};

	struct Vertex_3P3N3T2Tc
	{
		Vec3 m_Position;
		Vec3 m_Normal;
		Vec3 m_Tangent;
		Vec2 m_TexCoord;

		Vertex_3P3N3T2Tc() = default;

		Vertex_3P3N3T2Tc(Vec3 pos, Vec3 normal, Vec3 tangent, Vec2 texCoord)
			: m_Position(pos), m_Normal(normal), m_Tangent(tangent), m_TexCoord(texCoord) {}
	};
}

namespace CKE {
	class MeshResource : public IResource
	{
		friend MeshLoader;

	public:
		inline Vector<Vertex_3P3N3T2Tc> const& GetVertices() const { return m_Vertices; }
		inline Vector<u32> const&              GetIndices() const { return m_Indices; }

		inline BufferHandle const& GetVertexBuffer() const { return m_VertexBufferHandle; }
		inline BufferHandle const& GetIndexBuffer() const { return m_IndexBufferHandle; }

	private:
		// Triangle Mesh Data
		Vector<Vertex_3P3N3T2Tc> m_Vertices;
		Vector<u32>              m_Indices;

		// Render Resources
		BufferHandle m_VertexBufferHandle;
		BufferHandle m_IndexBufferHandle;
	};
}
