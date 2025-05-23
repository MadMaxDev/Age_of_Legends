using UnityEngine;
using System.Collections;

[AddComponentMenu("2D Toolkit/Sprite/tk2dSprite")]
[RequireComponent(typeof(MeshRenderer))]
[RequireComponent(typeof(MeshFilter))]
[ExecuteInEditMode]
/// <summary>
/// Sprite implementation which maintains its own Unity Mesh. Leverages dynamic batching.
/// </summary>
public class tk2dSprite : tk2dBaseSprite
{
	Mesh mesh;
	Vector3[] meshVertices;
	Vector3[] meshNormals = null;
	Vector4[] meshTangents = null;
	Color[] meshColors;
	
	void Awake()
	{
		// Create mesh, independently to everything else
		mesh = new Mesh();
		GetComponent<MeshFilter>().mesh = mesh;
		
		// This will not be set when instantiating in code
		// In that case, Build will need to be called
		if (collection)
		{
			// reset spriteId if outside bounds
			// this is when the sprite collection data is corrupt
			if (_spriteId < 0 || _spriteId >= collection.Count)
				_spriteId = 0;
			
			Build();
		}
	}
	
	protected void OnDestroy()
	{
		if (mesh)
		{
#if UNITY_EDITOR
			DestroyImmediate(mesh);
#else
			Destroy(mesh);
#endif
		}
		
		if (meshColliderMesh)
		{
#if UNITY_EDITOR
			DestroyImmediate(meshColliderMesh);
#else
			Destroy(meshColliderMesh);
#endif
		}
	}
	
	public override void Build()
	{
		var sprite = collection.spriteDefinitions[spriteId];

		meshVertices = new Vector3[sprite.positions.Length];
        meshColors = new Color[sprite.positions.Length];
		
		meshNormals = new Vector3[0];
		meshTangents = new Vector4[0];
		
		if (sprite.normals != null && sprite.normals.Length > 0)
		{
			meshNormals = new Vector3[sprite.normals.Length];
		}
		if (sprite.tangents != null && sprite.tangents.Length > 0)
		{
			meshTangents = new Vector4[sprite.tangents.Length];
		}
		
		SetPositions(meshVertices, meshNormals, meshTangents);
		SetColors(meshColors);
		
		if (mesh == null)
		{
			mesh = new Mesh();
			GetComponent<MeshFilter>().mesh = mesh;
		}
		
		mesh.Clear();
		mesh.vertices = meshVertices;
		mesh.normals = meshNormals;
		mesh.tangents = meshTangents;
		mesh.colors = meshColors;
		mesh.uv = sprite.uvs;
		mesh.triangles = sprite.indices;
		
		UpdateMaterial();
		CreateCollider();
	}
	
	protected override void UpdateGeometry() { UpdateGeometryImpl(); }
	protected override void UpdateColors() { UpdateColorsImpl(); }
	protected override void UpdateVertices() { UpdateVerticesImpl(); }
	
	
	protected void UpdateColorsImpl()
	{
#if UNITY_EDITOR
		// This can happen with prefabs in the inspector
		if (meshColors == null || meshColors.Length == 0)
			return;
#endif
		
		SetColors(meshColors);
		mesh.colors = meshColors;
	}
	
	protected void UpdateVerticesImpl()
	{
		var sprite = collection.spriteDefinitions[spriteId];
		
#if UNITY_EDITOR
		// This can happen with prefabs in the inspector
		if (meshVertices == null || meshVertices.Length == 0)
			return;
#endif
		
		// Clear out normals and tangents when switching from a sprite with them to one without
		if (sprite.normals.Length != meshNormals.Length)
		{
			meshNormals = (sprite.normals != null && sprite.normals.Length > 0)?(new Vector3[sprite.normals.Length]):(new Vector3[0]);
		}
		if (sprite.tangents.Length != meshTangents.Length)
		{
			meshTangents = (sprite.tangents != null && sprite.tangents.Length > 0)?(new Vector4[sprite.tangents.Length]):(new Vector4[0]);
		}
		
		SetPositions(meshVertices, meshNormals, meshTangents);
		mesh.vertices = meshVertices;
		mesh.normals = meshNormals;
		mesh.tangents = meshTangents;
		mesh.uv = sprite.uvs;
		mesh.bounds = GetBounds();
	}

	protected void UpdateGeometryImpl()
	{
#if UNITY_EDITOR
		// This can happen with prefabs in the inspector
		if (mesh == null)
			return;
#else
		if (mesh == null)
			Build();
#endif
		
		var sprite = collection.spriteDefinitions[spriteId];
		if (meshVertices.Length != sprite.positions.Length)
		{
			meshVertices = new Vector3[sprite.positions.Length];
			meshNormals = (sprite.normals != null && sprite.normals.Length > 0)?(new Vector3[sprite.normals.Length]):(new Vector3[0]);
			meshTangents = (sprite.tangents != null && sprite.tangents.Length > 0)?(new Vector4[sprite.tangents.Length]):(new Vector4[0]);
			meshColors = new Color[sprite.positions.Length];
		}
		SetPositions(meshVertices, meshNormals, meshTangents);
		SetColors(meshColors);

		mesh.Clear();
		mesh.vertices = meshVertices;
		mesh.normals = meshNormals;
		mesh.tangents = meshTangents;
		mesh.colors = meshColors;
		mesh.uv = sprite.uvs;
		mesh.bounds = GetBounds();
        mesh.triangles = sprite.indices;
	}
	
	protected override void UpdateMaterial()
	{
		if (GetComponent<Renderer>().sharedMaterial != collection.spriteDefinitions[spriteId].material)
			GetComponent<Renderer>().material = collection.spriteDefinitions[spriteId].material;
	}
	
	protected override int GetCurrentVertexCount()
	{
#if UNITY_EDITOR
		if (meshVertices == null)
			return 0;
#else
		if (meshVertices == null)
			Build();
#endif
		// Really nasty bug here found by Andrew Welch.
		return meshVertices.Length;
	}
}
