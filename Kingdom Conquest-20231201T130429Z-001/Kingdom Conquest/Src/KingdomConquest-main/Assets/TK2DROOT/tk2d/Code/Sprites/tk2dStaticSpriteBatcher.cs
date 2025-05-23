using UnityEngine;
using System.Collections.Generic;

[System.Serializable]
public class tk2dBatchedSprite
{
	public string name = ""; // for editing
	public int spriteId = 0;
	public Quaternion rotation = Quaternion.identity;
	public Vector3 position = Vector3.zero;
	public Vector3 localScale = Vector3.one;
	public Color color = Color.white;
	public bool alwaysPixelPerfect = false;
}

[AddComponentMenu("2D Toolkit/Sprite/tk2dStaticSpriteBatcher")]
[RequireComponent(typeof(MeshRenderer))]
[RequireComponent(typeof(MeshFilter))]
[ExecuteInEditMode]
public class tk2dStaticSpriteBatcher : MonoBehaviour, tk2dRuntime.ISpriteCollectionForceBuild
{
	public static int CURRENT_VERSION = 1;
	
	public int version;
	public tk2dBatchedSprite[] batchedSprites = null;
	public tk2dSpriteCollectionData spriteCollection = null;
	Mesh mesh = null;
	Mesh colliderMesh = null;
	
	[SerializeField] Vector3 _scale = new Vector3(1.0f, 1.0f, 1.0f);
	
#if UNITY_EDITOR
	// This is not exposed to game, as the cost of rebuilding this data is very high
	public Vector3 scale
	{
		get { UpgradeData(); return _scale; }
		set
		{
			bool needBuild = _scale != value;
			_scale = value;
			if (needBuild)
				Build();
		}
	}
#endif
	
	void Awake()
	{
		// Create mesh, independently to everything else
		mesh = new Mesh();
		GetComponent<MeshFilter>().mesh = mesh;

		Build();
	}
	
	// Sanitize data, returns true if needs rebuild
	bool UpgradeData()
	{
		if (version == CURRENT_VERSION) return false;
		
		if (_scale == Vector3.zero) _scale = Vector3.one;
		version = CURRENT_VERSION;
		return true;
	}
	
	public void Build()
	{
		UpgradeData();
		
		if (mesh)
		{
			mesh.Clear();
		}
		
		if (colliderMesh)
		{
#if UNITY_EDITOR
			DestroyImmediate(colliderMesh);
#else
			Destroy(colliderMesh);
#endif
			colliderMesh = null;
		}
		
		if (!spriteCollection || batchedSprites == null || batchedSprites.Length == 0)
		{
		}
		else
		{
			BuildRenderMesh();
			BuildPhysicsMesh();
		}
	}
	
	void SortBatchedSprites()
	{
		List<tk2dBatchedSprite> solidBatches = new List<tk2dBatchedSprite>();
		List<tk2dBatchedSprite> otherBatches = new List<tk2dBatchedSprite>();
		foreach (var sprite in batchedSprites)
		{
			var spriteData = spriteCollection.spriteDefinitions[sprite.spriteId];
			if (spriteData.material.renderQueue == 2000)
				solidBatches.Add(sprite);
			else
				otherBatches.Add(sprite);
		}
		
		List<tk2dBatchedSprite> allBatches = new List<tk2dBatchedSprite>(solidBatches.Count + otherBatches.Count);
		allBatches.AddRange(solidBatches);
		allBatches.AddRange(otherBatches);
		batchedSprites = allBatches.ToArray();
	}

	void BuildRenderMesh()
	{
		List<Material> materials = new List<Material>();
		List<List<int>> indices = new List<List<int>>();
		
		int numVertices = 0;
		foreach (var sprite in batchedSprites) 
		{
			var spriteData = spriteCollection.spriteDefinitions[sprite.spriteId];
			numVertices += spriteData.positions.Length;
		}
		
		Vector3[] meshVertices = new Vector3[numVertices];
		Color[] meshColors = new Color[numVertices];
		Vector2[] meshUvs = new Vector2[numVertices];
		
		int currVertex = 0;
		int currIndex = 0;

		Material currentMaterial = null;
		List<int> currentIndices = null;
		
		SortBatchedSprites();
		
		foreach (var sprite in batchedSprites)
		{
			var spriteData = spriteCollection.spriteDefinitions[sprite.spriteId];
			
			if (spriteData.material != currentMaterial)
			{
				if (currentMaterial != null)
				{
					materials.Add(currentMaterial);
					indices.Add(currentIndices);
				}
				
				currentMaterial = spriteData.material;
				currentIndices = new List<int>();
			}
			
			Color color = sprite.color;
	        if (spriteCollection.premultipliedAlpha) { color.r *= color.a; color.g *= color.a; color.b *= color.a; }
			
			for (int i = 0; i < spriteData.indices.Length; ++i)
				currentIndices.Add(currVertex + spriteData.indices[i]);
			
			for (int i = 0; i < spriteData.positions.Length; ++i)
			{
				Vector3 pos = new Vector3(spriteData.positions[i].x * sprite.localScale.x,
								  		  spriteData.positions[i].y * sprite.localScale.y,
								 		  spriteData.positions[i].z * sprite.localScale.z);
				pos = sprite.rotation * pos;
				pos += sprite.position;
				pos = new Vector3(pos.x * _scale.x, pos.y * _scale.y, pos.z * _scale.z);
				meshVertices[currVertex + i] = pos;
				meshUvs[currVertex + i] = spriteData.uvs[i];
				meshColors[currVertex + i] = color;
			}
			
			currIndex += spriteData.indices.Length;
			currVertex += spriteData.positions.Length;
		}
		
		if (currentIndices != null)
		{
			materials.Add(currentMaterial);
			indices.Add(currentIndices);
		}
		
		if (mesh)
		{
			mesh.vertices = meshVertices;
	        mesh.uv = meshUvs;
	        mesh.colors = meshColors;
			
			mesh.subMeshCount = indices.Count;
			for (int i = 0; i < indices.Count; ++i)
				mesh.SetTriangles(indices[i].ToArray(), i);
			
			mesh.RecalculateBounds();
		}
		
		GetComponent<Renderer>().sharedMaterials = materials.ToArray();
	}
	
	void BuildPhysicsMesh()
	{
		MeshCollider meshCollider = GetComponent<MeshCollider>();
		if (meshCollider != null && GetComponent<Collider>() != meshCollider)
		{
			// Already has a collider
			return;
		}
		
		int numIndices = 0;
		int numVertices = 0;
		
		// first pass, count required vertices and indices
		foreach (var sprite in batchedSprites) 
		{
			var spriteData = spriteCollection.spriteDefinitions[sprite.spriteId];
			if (spriteData.colliderType == tk2dSpriteDefinition.ColliderType.Box)
			{
				numIndices += 6 * 4;
				numVertices += 8;
			}
			else if (spriteData.colliderType == tk2dSpriteDefinition.ColliderType.Mesh)
			{
				numIndices += spriteData.colliderIndicesFwd.Length;
				numVertices += spriteData.colliderVertices.Length;
			}
		}
		
		if (numIndices == 0)
		{
			if (colliderMesh)
			{
#if UNITY_EDTIOR
				DestroyImmediate(colliderMesh);
#else
				Destroy(colliderMesh);
#endif
			}
			
			return;
		}
		
		if (meshCollider == null)
			meshCollider = gameObject.AddComponent<MeshCollider>();
		if (colliderMesh == null)
			colliderMesh = new Mesh();
		colliderMesh.Clear();
		
		// second pass, build composite mesh
		int currVertex = 0;
		Vector3[] vertices = new Vector3[numVertices];
		int currIndex = 0;
		int[] indices = new int[numIndices];
		
		foreach (var sprite in batchedSprites) 
		{
			var spriteData = spriteCollection.spriteDefinitions[sprite.spriteId];
			if (spriteData.colliderType == tk2dSpriteDefinition.ColliderType.Box)
			{
				Vector3 origin = new Vector3(spriteData.colliderVertices[0].x * sprite.localScale.x, spriteData.colliderVertices[0].y * sprite.localScale.y, spriteData.colliderVertices[0].z * sprite.localScale.z);
				Vector3 extents = new Vector3(spriteData.colliderVertices[1].x * sprite.localScale.x, spriteData.colliderVertices[1].y * sprite.localScale.y, spriteData.colliderVertices[1].z * sprite.localScale.z);
				Vector3 min = origin - extents;
				Vector3 max = origin + extents;
				
				vertices[currVertex + 0] = sprite.rotation * new Vector3(min.x, min.y, min.z) + sprite.position;
				vertices[currVertex + 1] = sprite.rotation * new Vector3(min.x, min.y, max.z) + sprite.position;
				vertices[currVertex + 2] = sprite.rotation * new Vector3(max.x, min.y, min.z) + sprite.position;
				vertices[currVertex + 3] = sprite.rotation * new Vector3(max.x, min.y, max.z) + sprite.position;
				vertices[currVertex + 4] = sprite.rotation * new Vector3(min.x, max.y, min.z) + sprite.position;
				vertices[currVertex + 5] = sprite.rotation * new Vector3(min.x, max.y, max.z) + sprite.position;
				vertices[currVertex + 6] = sprite.rotation * new Vector3(max.x, max.y, min.z) + sprite.position;
				vertices[currVertex + 7] = sprite.rotation * new Vector3(max.x, max.y, max.z) + sprite.position;
				
				for (int j = 0; j < 8; ++j)
				{
					Vector3 v = vertices[currVertex + j];
					v = new Vector3(v.x * _scale.x, v.y * _scale.y, v.z * _scale.z);
					vertices[currVertex + j] = v;
				}
				
				int[] indicesBack = { 0, 1, 2, 2, 1, 3, 6, 5, 4, 7, 5, 6, 3, 7, 6, 2, 3, 6, 4, 5, 1, 4, 1, 0 };
				int[] indicesFwd = { 2, 1, 0, 3, 1, 2, 4, 5, 6, 6, 5, 7, 6, 7, 3, 6, 3, 2, 1, 5, 4, 0, 1, 4 };
				
				float scl = sprite.localScale.x * sprite.localScale.y * sprite.localScale.z;
				int[] srcIndices = (scl >= 0)?indicesFwd:indicesBack;
				
				for (int i = 0; i < srcIndices.Length; ++i)
					indices[currIndex + i] = currVertex + srcIndices[i];
					
				currIndex += 6 * 4;
				currVertex += 8;
			}
			else if (spriteData.colliderType == tk2dSpriteDefinition.ColliderType.Mesh)
			{
				for (int i = 0; i < spriteData.colliderVertices.Length; ++i)
				{
					Vector3 pos = new Vector3(spriteData.colliderVertices[i].x * sprite.localScale.x,
											  spriteData.colliderVertices[i].y * sprite.localScale.y,
											  spriteData.colliderVertices[i].z * sprite.localScale.z);
					pos = sprite.rotation * pos;
					pos += sprite.position;
					pos = new Vector3(pos.x * _scale.x, pos.y * _scale.y, pos.z * _scale.z);
					
					vertices[currVertex + i] = pos;
				}
				
				float scl = sprite.localScale.x * sprite.localScale.y * sprite.localScale.z;
				int[] srcIndices = (scl >= 0)?spriteData.colliderIndicesFwd:spriteData.colliderIndicesBack;
				
				for (int i = 0; i < srcIndices.Length; ++i)
				{
					indices[currIndex + i] = currVertex + srcIndices[i];
				}
				
				currIndex += spriteData.colliderIndicesFwd.Length;
				currVertex += spriteData.colliderVertices.Length;
			}
		}
		
		colliderMesh.vertices = vertices;
		colliderMesh.triangles = indices;
		
		meshCollider.sharedMesh = colliderMesh;
	}
	
	public bool UsesSpriteCollection(tk2dSpriteCollectionData spriteCollection)
	{
		return this.spriteCollection == spriteCollection;	
	}
	
	public void ForceBuild()
	{
		Build();
	}
}


