using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.IO;

public class tk2dSpriteCollectionBuilder
{
	class SpriteLut
	{
		public int source; // index into source texture list, will only have multiple entries with same source, when splitting
		public Texture2D sourceTex;
		public Texture2D tex; // texture to atlas
		
		public bool isSplit; // is this part of a split?
		public int rx, ry, rw, rh; // split rectangle in texture coords
		
		public bool isDuplicate; // is this a duplicate texture?
		public int atlasIndex; // index in the atlas
		
		public bool isFont;
		public int fontId;
		public int charId;
	}
	
	public static void ResetCurrentBuild()
	{
		currentBuild = null;
	}
	
	/// <summary>
	/// Determines whether this texture is used as a sprite in a sprite collection
	/// </summary>
	public static bool IsSpriteSourceTexture(string path)
	{
		// This should only take existing indices, as we don't want to slow anything down here
		tk2dIndex index = tk2dEditorUtility.GetExistingIndex();
		if (index == null)
			return false;

		tk2dSpriteCollectionIndex[] scg = index.GetSpriteCollectionIndex();
		if (scg == null)
			return false;

        foreach (tk2dSpriteCollectionIndex thisScg in scg)
        {
			foreach (var textureGUID in thisScg.spriteTextureGUIDs)
			{
				if (textureGUID == AssetDatabase.AssetPathToGUID(path))
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	public static bool IsTextureImporterSetUp(string assetPath)
	{
        TextureImporter importer = (TextureImporter)TextureImporter.GetAtPath(assetPath);
        if (importer.textureType != TextureImporterType.Default ||
            importer.textureFormat != TextureImporterFormat.AutomaticTruecolor ||
            importer.npotScale != TextureImporterNPOTScale.None ||
            importer.isReadable != true ||
		    importer.maxTextureSize < 4096)
		{
			return false;
		}	
		return true;
	}
	
	public static bool ConfigureSpriteTextureImporter(string assetPath)
	{
		// make sure the source texture is npot and readable, and uncompressed
        TextureImporter importer = (TextureImporter)TextureImporter.GetAtPath(assetPath);
        if (importer.textureType != TextureImporterType.Default ||
            importer.textureFormat != TextureImporterFormat.AutomaticTruecolor ||
            importer.npotScale != TextureImporterNPOTScale.None ||
            importer.isReadable != true ||
		    importer.maxTextureSize < 4096)
        {
            importer.textureFormat = TextureImporterFormat.AutomaticTruecolor;
            importer.textureType = TextureImporterType.Default;
            importer.npotScale = TextureImporterNPOTScale.None;
            importer.isReadable = true;
			importer.mipmapEnabled = false;
			importer.maxTextureSize = 4096;

			return true;
        }
		
		return false;
	}

	// Rebuild a sprite collection when out of date
	// Identifies changed textures by comparing GUID
	public static void RebuildOutOfDate(string[] changedPaths)
    {
		// This should only take existing indices, as we don't want to slow anything down here
		tk2dIndex index = tk2dEditorUtility.GetExistingIndex();
		if (index == null)
			return;

		tk2dSpriteCollectionIndex[] scg = index.GetSpriteCollectionIndex();
		if (scg == null)
			return;

        foreach (tk2dSpriteCollectionIndex thisScg in scg)
        {
			bool needRebuild = false;
			foreach (var textureGUID in thisScg.spriteTextureGUIDs)
			{
				foreach (string changedPath in changedPaths)
				{
					if (textureGUID == AssetDatabase.AssetPathToGUID(changedPath))
					{
						needRebuild = true;
						break;
					}
				}

				if (needRebuild)
				{
					break;
				}
			}

            if (needRebuild)
            {
				tk2dSpriteCollection spriteCollectionSource = AssetDatabase.LoadAssetAtPath( AssetDatabase.GUIDToAssetPath(thisScg.spriteCollectionGUID), typeof(tk2dSpriteCollection) ) as tk2dSpriteCollection;
				if (spriteCollectionSource != null)
				{
                	Rebuild(spriteCollectionSource);
				}

				tk2dEditorUtility.UnloadUnusedAssets();
            }
        }
    }
	
	static int defaultPad = 2;
	
	static int GetPadAmount(tk2dSpriteCollection gen, int spriteId)
	{
		int basePadAmount = 0;
		
		if (gen.padAmount == -1) basePadAmount = (gen.pixelPerfectPointSampled)?0:defaultPad;
		else basePadAmount = gen.padAmount;
		
		if (spriteId >= 0)
			basePadAmount += (gen.textureParams[spriteId].extraPadding==-1)?0:gen.textureParams[spriteId].extraPadding;
		
		return Mathf.Max(0, basePadAmount);
	}

	static void PadTexture(Texture2D tex, int pad, bool stretchPad)
	{
		Color bgColor = new Color(0,0,0,0);

		for (int y = 0; y < pad; ++y)
		{
			for (int x = 0; x < tex.width; ++x)
			{
				tex.SetPixel(x, y, stretchPad?tex.GetPixel(x, pad):bgColor);
				tex.SetPixel(x, tex.height - 1 - y, stretchPad?tex.GetPixel(x, tex.height - 1 - pad):bgColor);
			}
		}
		for (int x = 0; x < pad; ++x)
		{
			for (int y = 0; y < tex.height; ++y)
			{
				tex.SetPixel(x, y, stretchPad?tex.GetPixel(pad, y):bgColor);
				tex.SetPixel(tex.width - 1 - x, y, stretchPad?tex.GetPixel(tex.width - 1 - pad, y):bgColor);
			}
		}
	}

	static bool CheckAndFixUpParams(tk2dSpriteCollection gen)
	{
		if (gen.textureRefs != null && gen.textureParams != null && gen.textureRefs.Length != gen.textureParams.Length)
        {
			tk2dSpriteCollectionDefinition[] newDefs = new tk2dSpriteCollectionDefinition[gen.textureRefs.Length];
			int c = Mathf.Min( newDefs.Length, gen.textureParams.Length );

			if (gen.textureRefs.Length > gen.textureParams.Length)
			{
				Texture2D[] newTexRefs = new Texture2D[gen.textureRefs.Length - gen.textureParams.Length];
				System.Array.Copy(gen.textureRefs, gen.textureParams.Length, newTexRefs, 0, newTexRefs.Length);
				System.Array.Sort(newTexRefs, (Texture2D a, Texture2D b) => tk2dSpriteGuiUtility.NameCompare(a?a.name:"", b?b.name:""));
				System.Array.Copy(newTexRefs, 0, gen.textureRefs, gen.textureParams.Length, newTexRefs.Length);
			}

			for (int i = 0; i < c; ++i)
			{
				newDefs[i] = new tk2dSpriteCollectionDefinition();
				newDefs[i].CopyFrom( gen.textureParams[i] );
			}
			for (int i = c; i < newDefs.Length; ++i)
			{
				newDefs[i] = new tk2dSpriteCollectionDefinition();
				newDefs[i].pad = gen.defaults.pad;
				newDefs[i].additive = gen.defaults.additive;
				newDefs[i].anchor = gen.defaults.anchor;
				newDefs[i].scale = gen.defaults.scale;
				newDefs[i].colliderType = gen.defaults.colliderType;
			}
			gen.textureParams = newDefs;
        }

		// clear thumbnails on build
		foreach (var param in gen.textureParams)
		{
			param.thumbnailTexture = null;
		}

		foreach (var param in gen.textureParams)
		{
			if (gen.allowMultipleAtlases && param.dice)
			{
				EditorUtility.DisplayDialog("Error",
				                            "Multiple atlas spanning is not allowed when there are textures with dicing enabled in the SpriteCollection.",
							                "Ok");

				gen.allowMultipleAtlases = false;

				return false;
			}
		}

		return true;
	}

	static void SetUpSourceTextureFormats(tk2dSpriteCollection gen)
	{
		// make sure all textures are in the right format
		int numTexturesReimported = 0;
		List<Texture2D> texturesToProcess = new List<Texture2D>();
		
		for (int i = 0; i < gen.textureParams.Length; ++i)
		{
			if (gen.textureRefs[i] != null)
			{
				texturesToProcess.Add(gen.textureRefs[i]);
			}
		}
		if (gen.spriteSheets != null)
		{
			foreach (var v in gen.spriteSheets)
			{
				if (v.texture != null)
					texturesToProcess.Add(v.texture);
			}
		}
		if (gen.fonts != null)
		{
			foreach (var v in gen.fonts)
			{
				if (v.active || v.texture != null)
					texturesToProcess.Add(v.texture);
			}
		}
		foreach (var tex in texturesToProcess)
		{
			string thisTexturePath = AssetDatabase.GetAssetPath(tex);
			if (ConfigureSpriteTextureImporter(thisTexturePath))
			{
				numTexturesReimported++;
	            AssetDatabase.ImportAsset(thisTexturePath);
			}
		}
		if (numTexturesReimported > 0)
		{
			AssetDatabase.SaveAssets();
			AssetDatabase.Refresh();
		}
	}

	static Texture2D ProcessTexture(tk2dSpriteCollection settings, bool additive, bool stretchPad, Texture2D srcTex, int sx, int sy, int tw, int th, ref SpriteLut spriteLut, int padAmount)
	{
		// Can't have additive without premultiplied alpha
		if (!settings.premultipliedAlpha) additive = false;
		bool allowTrimming = !settings.disableTrimming;
		var textureCompression = settings.textureCompression;
		
		int[] ww = new int[tw];
		int[] hh = new int[th];
		for (int x = 0; x < tw; ++x) ww[x] = 0;
		for (int y = 0; y < th; ++y) hh[y] = 0;
		int numNotTransparent = 0;
		for (int x = 0; x < tw; ++x)
		{
			for (int y = 0; y < th; ++y)
			{
				Color col = srcTex.GetPixel(sx + x, sy + y);
				if (col.a > 0)
				{
					ww[x] = 1;
					hh[y] = 1;
					numNotTransparent++;
				}
			}
		}

		if (numNotTransparent > 0)
		{
			int x0 = 0, x1 = 0, y0 = 0, y1 = 0;

			// For custom geometry, use the bounds of the geometry
			var textureParams = settings.textureParams[spriteLut.source];
			if (textureParams.customSpriteGeometry)
			{
				x0 = int.MaxValue;
				y0 = int.MaxValue;
				x1 = -1;
				y1 = -1;
				
				foreach (var island in textureParams.geometryIslands)
				{
					foreach (var vert in island.points)
					{
						int minX = Mathf.FloorToInt(vert.x);
						int maxX = Mathf.CeilToInt(vert.x);
						float y = th - 1 - vert.y;
						int minY = Mathf.FloorToInt(y);
						int maxY = Mathf.CeilToInt(y);
						
						x0 = Mathf.Min(x0, minX);
						y0 = Mathf.Min(y0, minY);
						x1 = Mathf.Max(x1, maxX);
						y1 = Mathf.Max(y1, maxY);
					}
				}
			}
			else
			{
				for (int x = 0; x < tw; ++x) if (ww[x] == 1) { x0 = x; break; }
				for (int x = tw - 1; x >= 0; --x) if (ww[x] == 1) { x1 = x; break; }
				for (int y = 0; y < th; ++y) if (hh[y] == 1) { y0 = y; break; }
				for (int y = th - 1; y >= 0; --y) if (hh[y] == 1) { y1 = y; break; }
			}
			
			x1 = Mathf.Min(x1, tw - 1);
			y1 = Mathf.Min(y1, th - 1);
	
			int w1 = x1 - x0 + 1;
			int h1 = y1 - y0 + 1;
			
			if (!allowTrimming)
			{
				x0 = 0;
				y0 = 0;
				w1 = tw;
				h1 = th;
			}
			
			Texture2D dtex = new Texture2D(w1 + padAmount * 2, h1 + padAmount * 2);
			for (int x = 0; x < w1; ++x)
			{
				for (int y = 0; y < h1; ++y)
				{
					Color col = srcTex.GetPixel(sx + x0 + x, sy + y0 + y);
					dtex.SetPixel(x + padAmount, y + padAmount, col);
				}
			}
			
			if (settings.premultipliedAlpha)
			{
				for (int x = 0; x < dtex.width; ++x)
				{
					for (int y = 0; y < dtex.height; ++y)
					{
						Color col = dtex.GetPixel(x, y);
                        col.r *= col.a; col.g *= col.a; col.b *= col.a;
						col.a = additive?0.0f:col.a;
						dtex.SetPixel(x, y, col);
					}
				}
			}
			
			PadTexture(dtex, padAmount, stretchPad);
			switch (textureCompression)
			{
			case tk2dSpriteCollection.TextureCompression.Dithered16Bit_4444:
				tk2dEditor.TextureProcessing.FloydSteinbergDithering.DitherTexture(dtex, TextureFormat.ARGB4444, 0, 0, dtex.width, dtex.height); break;
			case tk2dSpriteCollection.TextureCompression.Dithered16Bit_565:
				tk2dEditor.TextureProcessing.FloydSteinbergDithering.DitherTexture(dtex, TextureFormat.RGB565, 0, 0, dtex.width, dtex.height); break;
			}
			dtex.Apply();

			spriteLut.rx = sx + x0;
			spriteLut.ry = sy + y0;
			spriteLut.rw = w1;
			spriteLut.rh = h1;
			spriteLut.tex = dtex;

			return dtex;
		}
		else
		{
			return null;
		}
	}

	static void TrimTextureList(tk2dSpriteCollection gen)
	{
		// trim textureRefs & textureParams
		int lastNonEmpty = -1;
		for (int i = 0; i < gen.textureRefs.Length; ++i)
		{
			if (gen.textureRefs[i] != null) lastNonEmpty = i;
		}
		System.Array.Resize(ref gen.textureRefs, lastNonEmpty + 1);
		System.Array.Resize(ref gen.textureParams, lastNonEmpty + 1);
	}
	
	static bool SetUpSpriteSheets(tk2dSpriteCollection gen)
	{
		// delete textures which aren't in sprite sheets any more
		// and delete textures which are out of range of the spritesheet
		for (int i = 0; i < gen.textureRefs.Length; ++i)
		{
			if (gen.textureParams[i].fromSpriteSheet)
			{
				bool found = false;
				foreach (var ss in gen.spriteSheets)
				{
					if (gen.textureRefs[i] == ss.texture)
					{
						found = true;
						int numTiles = (ss.numTiles == 0)?(ss.tilesX * ss.tilesY):Mathf.Min(ss.numTiles, ss.tilesX * ss.tilesY);
						// delete textures which are out of range
						if (gen.textureParams[i].regionId >= numTiles)
						{
							gen.textureRefs[i] = null;
							gen.textureParams[i].fromSpriteSheet = false;
							gen.textureParams[i].extractRegion = false;
							gen.textureParams[i].colliderType = tk2dSpriteCollectionDefinition.ColliderType.None;
							gen.textureParams[i].boxColliderMin = Vector3.zero;
							gen.textureParams[i].boxColliderMax = Vector3.zero;
						}
					}
				}

				if (!found)
				{
					gen.textureRefs[i] = null;
					gen.textureParams[i].fromSpriteSheet = false;
					gen.textureParams[i].extractRegion = false;
					gen.textureParams[i].colliderType = tk2dSpriteCollectionDefinition.ColliderType.None;
					gen.textureParams[i].boxColliderMin = Vector3.zero;
					gen.textureParams[i].boxColliderMax = Vector3.zero;
				}
			}
		}

		if (gen.spriteSheets == null)
		{
			gen.spriteSheets = new tk2dSpriteSheetSource[0];
		}
		
		int spriteSheetId = 0;
		for (spriteSheetId = 0; spriteSheetId < gen.spriteSheets.Length; ++spriteSheetId)
		{
			var spriteSheet = gen.spriteSheets[spriteSheetId];
			
			// New mode sprite sheets have sprites already created
			if (spriteSheet.version > 0)
				continue;
			
			// Sanity check
			if (spriteSheet.texture == null)
			{
				continue; // deleted, safely ignore this
			}
			if (spriteSheet.tilesX * spriteSheet.tilesY == 0 ||
			    (spriteSheet.numTiles != 0 && spriteSheet.numTiles > spriteSheet.tilesX * spriteSheet.tilesY))
			{
				EditorUtility.DisplayDialog("Invalid sprite sheet",
				                            "Sprite sheet '" + spriteSheet.texture.name + "' has an invalid number of tiles",
				                            "Ok");
				return false;
			}
			if ((spriteSheet.texture.width % spriteSheet.tilesX) != 0 || (spriteSheet.texture.height % spriteSheet.tilesY) != 0)
			{
				EditorUtility.DisplayDialog("Invalid sprite sheet",
				                            "Sprite sheet '" + spriteSheet.texture.name + "' doesn't match tile count",
				                            "Ok");
				return false;
			}

			int numTiles = (spriteSheet.numTiles == 0)?(spriteSheet.tilesX * spriteSheet.tilesY):Mathf.Min(spriteSheet.numTiles, spriteSheet.tilesX * spriteSheet.tilesY);
			for (int y = 0; y < spriteSheet.tilesY; ++y)
			{
				for (int x = 0; x < spriteSheet.tilesX; ++x)
				{
					// limit to number of tiles, if told to
					int tileIdx = y * spriteSheet.tilesX + x;
					if (tileIdx >= numTiles)
						break;
					
					bool foundInCollection = false;
					
					// find texture in collection
					int textureIdx = -1;
					for (int i = 0; i < gen.textureParams.Length; ++i)
					{
						if (gen.textureParams[i].fromSpriteSheet
						    && gen.textureParams[i].regionId == tileIdx
						    && gen.textureRefs[i] == spriteSheet.texture)
						{
							textureIdx = i;
							foundInCollection = true;
							break;
						}
					}

					if (textureIdx == -1)
					{
						// find first empty texture slot
						for (int i = 0; i < gen.textureParams.Length; ++i)
						{
							if (gen.textureRefs[i] == null)
							{
								textureIdx = i;
								break;
							}
						}
					}

					if (textureIdx == -1)
					{
						// texture not found, so extend arrays
						System.Array.Resize(ref gen.textureRefs, gen.textureRefs.Length + 1);
						System.Array.Resize(ref gen.textureParams, gen.textureParams.Length + 1);
						textureIdx = gen.textureRefs.Length - 1;
					}
					
					gen.textureRefs[textureIdx] = spriteSheet.texture;
					var param = new tk2dSpriteCollectionDefinition();
					param.fromSpriteSheet = true;
					param.name = spriteSheet.texture.name + "/" + tileIdx;
					param.regionId = tileIdx;
					param.regionW = spriteSheet.texture.width / spriteSheet.tilesX;
					param.regionH = spriteSheet.texture.height / spriteSheet.tilesY;
					param.regionX = (tileIdx % spriteSheet.tilesX) * param.regionW;
					param.regionY = (spriteSheet.tilesY - 1 - (tileIdx / spriteSheet.tilesX)) * param.regionH;
					param.extractRegion = true;
					param.additive = spriteSheet.additive;

					param.pad = spriteSheet.pad;
					param.anchor = (tk2dSpriteCollectionDefinition.Anchor)spriteSheet.anchor;
					param.scale = (spriteSheet.scale.sqrMagnitude == 0.0f)?Vector3.one:spriteSheet.scale;
					
					// Let the user tweak individually
					if (foundInCollection)
					{
						param.colliderType = gen.textureParams[textureIdx].colliderType;
						param.boxColliderMin = gen.textureParams[textureIdx].boxColliderMin;
						param.boxColliderMax = gen.textureParams[textureIdx].boxColliderMax;
						param.polyColliderIslands = gen.textureParams[textureIdx].polyColliderIslands;
						param.colliderConvex = gen.textureParams[textureIdx].colliderConvex;
						param.colliderSmoothSphereCollisions = gen.textureParams[textureIdx].colliderSmoothSphereCollisions;
						param.colliderColor = gen.textureParams[textureIdx].colliderColor;
					}
					else
					{
						param.colliderType = spriteSheet.colliderType;
					}

					gen.textureParams[textureIdx] = param;
				}
			}
		}

		return true;
	}

	static tk2dSpriteCollection currentBuild = null;
	static Texture2D[] sourceTextures;

    public static bool Rebuild(tk2dSpriteCollection gen)
    {
		// avoid "recursive" build being triggered by texture watcher
		if (currentBuild != null)
			return false;

		currentBuild = gen;
		
		List<Texture2D> allocatedTextures = new List<Texture2D>();
        string path = AssetDatabase.GetAssetPath(gen);
		string subDirName = Path.GetDirectoryName( path.Substring(7) );
		if (subDirName.Length > 0) subDirName += "/";

		string dataDirFullPath = Application.dataPath + "/" + subDirName + Path.GetFileNameWithoutExtension(path) + "_Data";
		string dataDirName = "Assets/" + dataDirFullPath.Substring( Application.dataPath.Length + 1 ) + "/";
		
		if (gen.atlasTextures == null || gen.atlasTextures.Length == 0 ||
		    gen.atlasMaterials == null || gen.atlasMaterials.Length == 0 ||
			gen.altMaterials == null || gen.altMaterials.Length == 0 ||
		    gen.spriteCollection == null)
		{
			if (!Directory.Exists(dataDirFullPath)) Directory.CreateDirectory(dataDirFullPath);
			AssetDatabase.Refresh();
		}

		string prefabObjectPath = gen.spriteCollection?AssetDatabase.GetAssetPath(gen.spriteCollection):(dataDirName + "data.prefab");

      	if (!CheckAndFixUpParams(gen))
		{
			// Params failed check
			return false;
		}

		SetUpSourceTextureFormats(gen);
		SetUpSpriteSheets(gen);
		TrimTextureList(gen);

		// blank texture used when texture has been deleted
		Texture2D blankTexture = new Texture2D(2, 2);
		blankTexture.SetPixel(0, 0, Color.magenta);
		blankTexture.SetPixel(0, 1, Color.yellow);
		blankTexture.SetPixel(1, 0, Color.cyan);
		blankTexture.SetPixel(1, 1, Color.grey);
		blankTexture.Apply();
		
		// make local texture sources
		sourceTextures = new Texture2D[gen.textureRefs.Length];
		for (int i = 0; i < gen.textureParams.Length; ++i)
		{
			var param = gen.textureParams[i];
			if (param.extractRegion && gen.textureRefs[i] != null)
			{
				Texture2D localTex = new Texture2D(param.regionW, param.regionH);
				for (int y = 0; y < param.regionH; ++y)
				{
					for (int x = 0; x < param.regionW; ++x)
					{
						localTex.SetPixel(x, y, gen.textureRefs[i].GetPixel(param.regionX + x, param.regionY + y));
					}
				}
				localTex.name = gen.textureRefs[i].name + "/" + param.regionId.ToString();
				localTex.Apply();
				allocatedTextures.Add(localTex);
				sourceTextures[i] = localTex;
			}
			else
			{
				sourceTextures[i] = gen.textureRefs[i];
			}
		}

		// catalog all textures to atlas
		int numTexturesToAtlas = 0;
		List<SpriteLut> spriteLuts = new List<SpriteLut>();
		for (int i = 0; i < gen.textureParams.Length; ++i)
		{
			Texture2D currentTexture = sourceTextures[i];

			if (sourceTextures[i] == null)
			{
				gen.textureParams[i].dice = false;
				gen.textureParams[i].anchor = tk2dSpriteCollectionDefinition.Anchor.MiddleCenter;
				gen.textureParams[i].name = "";
				gen.textureParams[i].extractRegion = false;
				gen.textureParams[i].fromSpriteSheet = false;

				currentTexture = blankTexture;
			}
			else
			{
				if (gen.textureParams[i].name == null || gen.textureParams[i].name == "")
				{
					if (gen.textureParams[i].texture != currentTexture && !gen.textureParams[i].fromSpriteSheet)
					{
						gen.textureParams[i].name = currentTexture.name;
					}
				}
			}

			gen.textureParams[i].texture = currentTexture;


			if (gen.textureParams[i].dice)
			{
				// prepare to dice this up
				int diceUnitX = gen.textureParams[i].diceUnitX;
				int diceUnitY = gen.textureParams[i].diceUnitY;
				if (diceUnitX <= 0) diceUnitX = 128; // something sensible, please
				if (diceUnitY <= 0) diceUnitY = diceUnitX; // make square if not set

				Texture2D srcTex = currentTexture;
				for (int sx = 0; sx < srcTex.width; sx += diceUnitX)
				{
					for (int sy = 0; sy < srcTex.height; sy += diceUnitY)
					{
						int tw = Mathf.Min(diceUnitX, srcTex.width - sx);
						int th = Mathf.Min(diceUnitY, srcTex.height - sy);

						SpriteLut diceLut = new SpriteLut();
						diceLut.source = i;
						diceLut.isSplit = true;
						diceLut.sourceTex = srcTex;
						diceLut.isDuplicate = false; // duplicate diced textures can be chopped up differently, so don't detect dupes here

						Texture2D dest = ProcessTexture(gen, gen.textureParams[i].additive, true, srcTex, sx, sy, tw, th, ref diceLut, GetPadAmount(gen, i));
						if (dest)
						{
							diceLut.atlasIndex = numTexturesToAtlas++;
							spriteLuts.Add(diceLut);
						}
					}
				}
			}
			else
			{
				SpriteLut lut = new SpriteLut();
				lut.sourceTex = currentTexture;
				lut.source = i;

				lut.isSplit = false;
				lut.isDuplicate = false;
				for (int j = 0; j < spriteLuts.Count; ++j)
				{
					if (spriteLuts[j].sourceTex == lut.sourceTex)
					{
						lut.isDuplicate = true;
						lut.atlasIndex = spriteLuts[j].atlasIndex;
						lut.tex = spriteLuts[j].tex; // get old processed tex

						lut.rx = spriteLuts[j].rx; lut.ry = spriteLuts[j].ry;
						lut.rw = spriteLuts[j].rw; lut.rh = spriteLuts[j].rh;

						break;
					}
				}

				if (!lut.isDuplicate)
				{
					lut.atlasIndex = numTexturesToAtlas++;
					bool stretchPad = false;
					if (gen.textureParams[i].pad == tk2dSpriteCollectionDefinition.Pad.Extend) stretchPad = true;

					Texture2D dest = ProcessTexture(gen, gen.textureParams[i].additive, stretchPad, currentTexture, 0, 0, currentTexture.width, currentTexture.height, ref lut, GetPadAmount(gen, i));
					if (dest == null)
					{
						// fall back to a tiny blank texture
						lut.tex = new Texture2D(1, 1);
						lut.tex.SetPixel(0, 0, new Color( 0, 0, 0, 0 ));
						PadTexture(lut.tex, GetPadAmount(gen, i), stretchPad);
						lut.tex.Apply();

						lut.rx = currentTexture.width / 2; lut.ry = currentTexture.height / 2;
						lut.rw = 1; lut.rh = 1;
					}
				}

				spriteLuts.Add(lut);
			}
		}
		
		// Font
		Dictionary<tk2dSpriteCollectionFont, tk2dEditor.Font.Info> fontInfoDict = new Dictionary<tk2dSpriteCollectionFont, tk2dEditor.Font.Info>();
		if (gen.fonts != null)
		{
			for (int i = 0; i < gen.fonts.Length; ++i)
			{
				var font = gen.fonts[i];
				if (!font.InUse) continue;
				
				var fontInfo = tk2dEditor.Font.Builder.ParseBMFont( AssetDatabase.GetAssetPath(font.bmFont) );
				fontInfoDict[font] = fontInfo;
				foreach (var c in fontInfo.chars)
				{
					// skip empty textures
					if (c.width <= 0 || c.height <= 0)
						continue;
					
					SpriteLut lut = new SpriteLut();
					
					int cy = font.flipTextureY?c.y:(fontInfo.scaleH - c.y - c.height);
					Texture2D dest = ProcessTexture(gen, false, false, 
						font.texture, c.x, cy, c.width, c.height, 
						ref lut, GetPadAmount(gen, -1));
					if (dest == null)
					{
						// probably fully transparent
						continue;
					}
					
					lut.sourceTex = dest;
					lut.tex = dest;
					lut.source = -1;
					lut.isFont = true;
					lut.isDuplicate = false;
					lut.fontId = i;
					lut.charId = c.id;
					lut.rx = lut.rx - c.x;
					lut.ry = lut.ry - cy;
					
					lut.atlasIndex = numTexturesToAtlas++;
					
					spriteLuts.Add(lut);
				}
				
				// Add one blank char for fallbacks
				{
					int dims = 5;
					SpriteLut lut = new SpriteLut();
					lut.tex = new Texture2D(dims, dims);
					for (int y = 0; y < dims; ++y)
						for (int x = 0; x < dims; ++x)
							lut.tex.SetPixel(x, y, Color.clear);
					lut.tex.Apply();
					lut.sourceTex = lut.tex;
					lut.isFont = true;
					lut.isDuplicate = false;
					lut.fontId = i;
					lut.charId = -1;
					lut.atlasIndex = numTexturesToAtlas++;
					
					spriteLuts.Add(lut);
				}
			}
		}

        // Create texture
		Texture2D[] textureList = new Texture2D[numTexturesToAtlas];
        int titer = 0;
        for (int i = 0; i < spriteLuts.Count; ++i)
        {
			SpriteLut _lut = spriteLuts[i];
			if (!_lut.isDuplicate)
			{
				textureList[titer++] = _lut.tex;
			}
        }
		
		// Build atlas
		bool forceAtlasSize = gen.forceTextureSize;
		int atlasWidth = forceAtlasSize?gen.forcedTextureWidth:gen.maxTextureSize;
		int atlasHeight = forceAtlasSize?gen.forcedTextureHeight:gen.maxTextureSize;
		bool forceSquareAtlas = forceAtlasSize?false:gen.forceSquareAtlas;
		bool allowFindingOptimalSize = !forceAtlasSize;
		tk2dEditor.Atlas.Builder atlasBuilder = new tk2dEditor.Atlas.Builder(atlasWidth, atlasHeight, gen.allowMultipleAtlases?64:1, allowFindingOptimalSize, forceSquareAtlas);
		if (textureList.Length > 0)
		{
			foreach (Texture2D currTexture in textureList)
			{
				atlasBuilder.AddRect(currTexture.width, currTexture.height);
			}
			if (atlasBuilder.Build() != 0)
			{
				if (atlasBuilder.HasOversizeTextures())
				{
					EditorUtility.DisplayDialog("Unable to fit in atlas",
					                            "You have a texture which exceeds the atlas size." +
					                            "Consider putting it in a separate atlas, enabling dicing, or" +
					                            "reducing the texture size",
								                "Ok");
				}
				else
				{
					EditorUtility.DisplayDialog("Unable to fit textures in requested atlas area",
					                            "There are too many textures in this collection for the requested " +
					                            "atlas size.",
								                "Ok");
				}
				return false;
			}
		}
		
		// Fill atlas textures
		tk2dEditor.Atlas.Data[] atlasData = atlasBuilder.GetAtlasData();
		System.Array.Resize(ref gen.atlasTextures, atlasData.Length);
		System.Array.Resize(ref gen.atlasMaterials, atlasData.Length);
		if (atlasData.Length > 1)
		{
			// wipe out alt materials when atlas spanning is on
			gen.altMaterials = new Material[0];
		}
		for (int atlasIndex = 0; atlasIndex < atlasData.Length; ++atlasIndex)
		{
	        Texture2D tex = new Texture2D(64, 64, TextureFormat.ARGB32, false);

			gen.atlasWastage = (1.0f - atlasData[0].occupancy) * 100.0f;
			gen.atlasWidth = atlasData[0].width;
			gen.atlasHeight = atlasData[0].height;

			tex.Reinitialize(atlasData[atlasIndex].width, atlasData[atlasIndex].height);

			// Clear texture, unsure if this is REALLY necessary
			// Turns out it is
			for (int yy = 0; yy < tex.height; ++yy)
			{
				for (int xx = 0; xx < tex.width; ++xx)
				{
					tex.SetPixel(xx, yy, Color.clear);
				}
			}

			for (int i = 0; i < atlasData[atlasIndex].entries.Length; ++i)
			{
				var entry = atlasData[atlasIndex].entries[i];
				Texture2D source = textureList[entry.index];

				if (!entry.flipped)
				{
					for (int y = 0; y < source.height; ++y)
					{
						for (int x = 0; x < source.width; ++x)
						{
							tex.SetPixel(entry.x + x, entry.y + y, source.GetPixel(x, y));
						}
					}
				}
				else
				{
					for (int y = 0; y < source.height; ++y)
					{
						for (int x = 0; x < source.width; ++x)
						{
							tex.SetPixel(entry.x + y, entry.y + x, source.GetPixel(x, y));
						}
					}
				}
			}
			tex.Apply();

			string texturePath = gen.atlasTextures[atlasIndex]?AssetDatabase.GetAssetPath(gen.atlasTextures[atlasIndex]):(dataDirName + "atlas" + atlasIndex + ".png");

			// Write filled atlas to disk
			byte[] bytes = tex.EncodeToPNG();
			System.IO.FileStream fs = System.IO.File.Create(texturePath);
			fs.Write(bytes, 0, bytes.Length);
			fs.Close();

			AssetDatabase.SaveAssets();
			AssetDatabase.Refresh();
			Object.DestroyImmediate(tex);
			
			tex = AssetDatabase.LoadAssetAtPath(texturePath, typeof(Texture2D)) as Texture2D;
			gen.atlasTextures[atlasIndex] = tex;

			// Make sure texture is set up with the correct max size and compression type
			SetUpTargetTexture(gen, tex);

	        // Create material if necessary
	        if (gen.atlasMaterials[atlasIndex] == null)
	        {
				Material mat;
	            if (gen.premultipliedAlpha)
	                mat = new Material(Shader.Find("tk2d/PremulVertexColor"));
	            else
	                mat = new Material(Shader.Find("tk2d/BlendVertexColor"));

				mat.mainTexture = tex;

				string materialPath = gen.atlasMaterials[atlasIndex]?AssetDatabase.GetAssetPath(gen.atlasMaterials[atlasIndex]):(dataDirName + "atlas" + atlasIndex + "_material.mat");
	            AssetDatabase.CreateAsset(mat, materialPath);
				AssetDatabase.SaveAssets();

				gen.atlasMaterials[atlasIndex] = AssetDatabase.LoadAssetAtPath(materialPath, typeof(Material)) as Material;
			}
			
			// gen.altMaterials must either have length 0, or contain at least the material used in the game
			if (!gen.allowMultipleAtlases && (gen.altMaterials == null || gen.altMaterials.Length == 0))
				gen.altMaterials = new Material[1] { gen.atlasMaterials[0] };
		}

        // Create prefab
		if (gen.spriteCollection == null)
		{
			GameObject go = new GameObject();
			go.AddComponent<tk2dSpriteCollectionData>();
#if (UNITY_3_0 || UNITY_3_1 || UNITY_3_2 || UNITY_3_3 || UNITY_3_4)
			Object p = EditorUtility.CreateEmptyPrefab(prefabObjectPath);
			EditorUtility.ReplacePrefab(go, p);
#else
			Object p = PrefabUtility.CreateEmptyPrefab(prefabObjectPath);
			PrefabUtility.ReplacePrefab(go, p);
#endif
			GameObject.DestroyImmediate(go);
			AssetDatabase.SaveAssets();

			gen.spriteCollection = AssetDatabase.LoadAssetAtPath(prefabObjectPath, typeof(tk2dSpriteCollectionData)) as tk2dSpriteCollectionData;
		}

		tk2dSpriteCollectionData coll = gen.spriteCollection;
		coll.textures = new Texture[gen.atlasTextures.Length];
		for (int i = 0; i < gen.atlasTextures.Length; ++i)
		{
			coll.textures[i] = gen.atlasTextures[i];
		}
		
		if (gen.atlasMaterials.Length == 1)
		{
			coll.materials = new Material[gen.altMaterials.Length];
	        for (int i = 0; i < gen.altMaterials.Length; ++i)
				coll.materials[i] = gen.altMaterials[i];
		}
		else
		{
			coll.materials = new Material[gen.atlasMaterials.Length];
	        for (int i = 0; i < gen.atlasMaterials.Length; ++i)
				coll.materials[i] = gen.atlasMaterials[i];
		}
		
		// Wipe out legacy data
		coll.material = null;
		
        coll.premultipliedAlpha = gen.premultipliedAlpha;
        coll.spriteDefinitions = new tk2dSpriteDefinition[gen.textureParams.Length];
		coll.version = tk2dSpriteCollectionData.CURRENT_VERSION;
		coll.materialIdsValid = true;
		coll.spriteCollectionGUID = AssetDatabase.AssetPathToGUID(AssetDatabase.GetAssetPath(gen));
		coll.spriteCollectionName = gen.name;
		coll.invOrthoSize = 1.0f / gen.targetOrthoSize;
		coll.halfTargetHeight = gen.targetHeight * 0.5f;
		
		int buildKey = Random.Range(0, int.MaxValue);
		while (buildKey == coll.buildKey)
		{
			buildKey = Random.Range(0, int.MaxValue);
		}
		coll.buildKey = buildKey; // a random build number so we can identify changed collections quickly
		
		for (int i = 0; i < coll.spriteDefinitions.Length; ++i)
		{
			coll.spriteDefinitions[i] = new tk2dSpriteDefinition();
			if (gen.textureRefs[i])
			{
				string assetPath = AssetDatabase.GetAssetPath(gen.textureRefs[i]);
				string guid = AssetDatabase.AssetPathToGUID(assetPath);
				coll.spriteDefinitions[i].sourceTextureGUID = guid;
			}
			else
			{
				coll.spriteDefinitions[i].sourceTextureGUID = "";
			}
		
			coll.spriteDefinitions[i].extractRegion = gen.textureParams[i].extractRegion;
			coll.spriteDefinitions[i].regionX = gen.textureParams[i].regionX;
			coll.spriteDefinitions[i].regionY = gen.textureParams[i].regionY;
			coll.spriteDefinitions[i].regionW = gen.textureParams[i].regionW;
			coll.spriteDefinitions[i].regionH = gen.textureParams[i].regionH;
		}
		coll.allowMultipleAtlases = gen.allowMultipleAtlases;
		coll.dataGuid = AssetDatabase.AssetPathToGUID(AssetDatabase.GetAssetPath(coll));
		
		float scale = 1.0f;
		if (gen.useTk2dCamera)
		{
			scale = 1.0f;
		}
		else
		{
			scale = 2.0f * gen.targetOrthoSize / gen.targetHeight;
		}
		
		// Build fonts
		foreach (var font in fontInfoDict.Keys)
		{
			var fontInfo = fontInfoDict[font];
			List<SpriteLut> fontSpriteLut = new List<SpriteLut>();
			int fontId = 0;
			for (fontId = 0; fontId < gen.fonts.Length; ++fontId)
				if (gen.fonts[fontId] == font) break;
			foreach (var v in spriteLuts)
			{
				if (v.isFont && v.fontId == fontId)
					fontSpriteLut.Add(v);
			}
			
			fontInfo.scaleW = coll.textures[0].width;
			fontInfo.scaleH = coll.textures[0].height;
			
			// Build a local sprite lut only relevant to this font
			UpdateFontData(gen, scale, atlasData, fontSpriteLut, font, fontInfo);
			
			// Set material
			font.data.material = coll.materials[0];
			font.editorData.material = coll.materials[0];
			
			// Mark to save
			EditorUtility.SetDirty(font.data);
		}
		
		// Build textures
		UpdateVertexCache(gen, scale, atlasData, coll, spriteLuts);

		// Free tmp textures
		foreach (var sprite in spriteLuts)
		{
			if (!sprite.isDuplicate)
				Object.DestroyImmediate(sprite.tex);
		}
		foreach (var tex in allocatedTextures)
			Object.DestroyImmediate(tex);
	
        // refresh existing
		RefreshExistingAssets(gen.spriteCollection);
		
		// save changes
		var index = tk2dEditorUtility.GetOrCreateIndex();
		index.AddSpriteCollectionData(gen.spriteCollection);
		EditorUtility.SetDirty(gen.spriteCollection);
		EditorUtility.SetDirty(gen);

		sourceTextures = null; // need to clear, its static
		currentBuild = null;
		
		tk2dEditorUtility.GetOrCreateIndex().AddSpriteCollectionData(gen.spriteCollection);
		tk2dEditorUtility.CommitIndex();
	
		Object.DestroyImmediate(blankTexture);
		
		return true;
    }
	
	static void RefreshExistingAssets(tk2dSpriteCollectionData spriteCollectionData)
	{
		foreach (var assembly in System.AppDomain.CurrentDomain.GetAssemblies())
		{
			System.Type[] types = assembly.GetTypes();
			foreach (var type in types)
			{
				if (type.GetInterface("tk2dRuntime.ISpriteCollectionForceBuild") != null)
				{
					Object[] objects = Resources.FindObjectsOfTypeAll(type);
					foreach (var o in objects)
					{
						if (tk2dEditorUtility.IsPrefab(o))
							continue;
						
						tk2dRuntime.ISpriteCollectionForceBuild isc = o as tk2dRuntime.ISpriteCollectionForceBuild;
						if (isc.UsesSpriteCollection(spriteCollectionData))
							isc.ForceBuild();
					}
				}
			}
		}
	}

	static void SetUpTargetTexture(tk2dSpriteCollection gen, Texture2D tex)
	{
		bool textureDirty = false;

		string targetTexPath = AssetDatabase.GetAssetPath(tex);
		TextureImporter importer = (TextureImporter)TextureImporter.GetAtPath(targetTexPath);
		if (gen.maxTextureSize != importer.maxTextureSize)
		{
			importer.maxTextureSize = gen.maxTextureSize;
			textureDirty = true;
		}
		TextureImporterFormat targetFormat;
		switch (gen.textureCompression)
		{
		case tk2dSpriteCollection.TextureCompression.Uncompressed: targetFormat = TextureImporterFormat.AutomaticTruecolor; break;
		case tk2dSpriteCollection.TextureCompression.Reduced16Bit: targetFormat = TextureImporterFormat.Automatic16bit; break;
		case tk2dSpriteCollection.TextureCompression.Dithered16Bit_4444: targetFormat = TextureImporterFormat.Automatic16bit; break;
		case tk2dSpriteCollection.TextureCompression.Dithered16Bit_565: targetFormat = TextureImporterFormat.Automatic16bit; break;
		case tk2dSpriteCollection.TextureCompression.Compressed: targetFormat = TextureImporterFormat.AutomaticCompressed; break;

		default: targetFormat = TextureImporterFormat.AutomaticTruecolor; break;
		}

		if (targetFormat != importer.textureFormat)
		{
			importer.textureFormat = targetFormat;
			textureDirty = true;
		}

		// Make sure texture is set to point filtered when no pad mode is selected
		FilterMode targetFilterMode = gen.pixelPerfectPointSampled?FilterMode.Point:FilterMode.Bilinear;
		if (tex.filterMode != targetFilterMode)
		{
			importer.filterMode = targetFilterMode;
			EditorUtility.SetDirty(importer);
			textureDirty = true;
		}


		if (textureDirty)
		{
			EditorUtility.SetDirty(importer);
			AssetDatabase.ImportAsset(targetTexPath);
		}
	}
	
	static void UpdateFontData(tk2dSpriteCollection gen, float scale, tk2dEditor.Atlas.Data[] packers, List<SpriteLut> spriteLuts, tk2dSpriteCollectionFont font, tk2dEditor.Font.Info fontInfo)
	{
		Dictionary<int, SpriteLut> glyphLut = new Dictionary<int, SpriteLut>();
		List<SpriteLut> values = new List<SpriteLut>();
		foreach (var k in glyphLut.Keys) values.Add(glyphLut[k]);
		
		foreach (var v in spriteLuts)
			glyphLut[v.charId] = v;
		int padAmount = GetPadAmount(gen, -1);
		foreach (var c in fontInfo.chars)
		{
			if (glyphLut.ContainsKey(c.id))
			{
				var glyphSprite = glyphLut[c.id];
				var atlasEntry = packers[0].FindEntryWithIndex(glyphSprite.atlasIndex);
				
				c.texOffsetX = glyphSprite.rx;
				c.texOffsetY = glyphSprite.ry;
				
				c.texX = atlasEntry.x + padAmount;
				c.texY = atlasEntry.y + padAmount;
				c.texW = atlasEntry.w - padAmount * 2;
				c.texH = atlasEntry.h - padAmount * 2;
				c.texFlipped = atlasEntry.flipped;
			}
			else
			{
				var glyphSprite = glyphLut[-1];
				var atlasEntry = packers[0].FindEntryWithIndex(glyphSprite.atlasIndex);
				
				c.texOffsetX = 0;
				c.texOffsetY = 0;
				
				c.texX = atlasEntry.x + 1;
				c.texY = atlasEntry.y + 1;
				c.texW = 0;
				c.texH = 0;
				c.texFlipped = false;
			}
			c.texOverride = true;
		}
		
		tk2dEditor.Font.Builder.BuildFont(fontInfo, font.data, scale, font.charPadX, font.dupeCaps, false, null, 0);
	}
	
    static void UpdateVertexCache(tk2dSpriteCollection gen, float scale, tk2dEditor.Atlas.Data[] packers, tk2dSpriteCollectionData coll, List<SpriteLut> spriteLuts)
    {
        for (int i = 0; i < sourceTextures.Length; ++i)
        {
			SpriteLut _lut = null;
			for (int j = 0; j < spriteLuts.Count; ++j)
			{
				if (spriteLuts[j].source == i)
				{
					_lut = spriteLuts[j];
					break;
				}
			}
			
			int padAmount = GetPadAmount(gen, i);

            tk2dSpriteCollectionDefinition thisTexParam = gen.textureParams[i];
			tk2dEditor.Atlas.Data packer = null;
			tk2dEditor.Atlas.Entry atlasEntry = null;
			int atlasIndex = 0;
			foreach (var p in packers)
			{
				if ((atlasEntry = p.FindEntryWithIndex(_lut.atlasIndex)) != null)
				{
					packer = p;
					break;
				}
				++atlasIndex;
			}
			float fwidth = packer.width;
    	    float fheight = packer.height;

            int tx = atlasEntry.x + padAmount, ty = atlasEntry.y + padAmount, tw = atlasEntry.w - padAmount * 2, th = atlasEntry.h - padAmount * 2;
            int sd_y = packer.height - ty - th;

			float uvOffsetX = 0.001f / fwidth;
			float uvOffsetY = 0.001f / fheight;

            Vector2 v0 = new Vector2(tx / fwidth + uvOffsetX, 1.0f - (sd_y + th) / fheight + uvOffsetY);
            Vector2 v1 = new Vector2((tx + tw) / fwidth - uvOffsetX, 1.0f - sd_y / fheight - uvOffsetY);

            Mesh mesh = null;
            Transform meshTransform = null;
            GameObject instantiated = null;
			
			Vector3 colliderOrigin = new Vector3();

            if (thisTexParam.overrideMesh)
            {
				// Disabled
                instantiated = GameObject.Instantiate(thisTexParam.overrideMesh) as GameObject;
                MeshFilter meshFilter = instantiated.GetComponentInChildren<MeshFilter>();
                if (meshFilter == null)
                {
                    Debug.LogError("Unable to find mesh");
                    GameObject.DestroyImmediate(instantiated);
                }
                else
                {
                    mesh = meshFilter.sharedMesh;
                    meshTransform = meshFilter.gameObject.transform;
                }
            }
			
			Vector3 untrimmedPos0 = Vector3.zero, untrimmedPos1 = Vector3.one;
			
            if (mesh)
            {
                coll.spriteDefinitions[i].positions = new Vector3[mesh.vertices.Length];
                coll.spriteDefinitions[i].uvs = new Vector2[mesh.vertices.Length];
                for (int j = 0; j < mesh.vertices.Length; ++j)
                {
                    coll.spriteDefinitions[i].positions[j] = meshTransform.TransformPoint(mesh.vertices[j]);
                    coll.spriteDefinitions[i].uvs[j] = new Vector2(v0.x + (v1.x - v0.x) * mesh.uv[j].x, v0.y + (v1.y - v0.y) * mesh.uv[j].y);
                }
                coll.spriteDefinitions[i].indices = new int[mesh.triangles.Length];
                for (int j = 0; j < mesh.triangles.Length; ++j)
                {
                    coll.spriteDefinitions[i].indices[j] = mesh.triangles[j];
                }
                GameObject.DestroyImmediate(instantiated);
            }
            else
            {
				Texture2D thisTextureRef = sourceTextures[i];
				
				int texHeightI = thisTextureRef?thisTextureRef.height:2;
				int texWidthI = thisTextureRef?thisTextureRef.width:2;
				float texHeight = texHeightI;
       			float texWidth = texWidthI;

				float h = thisTextureRef?thisTextureRef.height:64;
				float w = thisTextureRef?thisTextureRef.width:64;
                h *= thisTexParam.scale.y;
                w *= thisTexParam.scale.x;

				float scaleX = w * scale;
                float scaleY = h * scale;
				
				// anchor coordinate system is (0, 0) = top left, to keep it the same as photoshop, etc.
                switch (thisTexParam.anchor)
                {
                    case tk2dSpriteCollectionDefinition.Anchor.LowerLeft: thisTexParam.anchorX = 0; thisTexParam.anchorY = texHeightI; break;
                    case tk2dSpriteCollectionDefinition.Anchor.LowerCenter: thisTexParam.anchorX = texWidthI / 2; thisTexParam.anchorY = texHeightI; break;
                    case tk2dSpriteCollectionDefinition.Anchor.LowerRight: thisTexParam.anchorX = texWidthI; thisTexParam.anchorY = texHeightI; break;

                    case tk2dSpriteCollectionDefinition.Anchor.MiddleLeft: thisTexParam.anchorX = 0; thisTexParam.anchorY = texHeightI / 2; break;
                    case tk2dSpriteCollectionDefinition.Anchor.MiddleCenter: thisTexParam.anchorX = texWidthI / 2; thisTexParam.anchorY = texHeightI / 2; break;
                    case tk2dSpriteCollectionDefinition.Anchor.MiddleRight: thisTexParam.anchorX = texWidthI; thisTexParam.anchorY = texHeightI / 2; break;

                    case tk2dSpriteCollectionDefinition.Anchor.UpperLeft: thisTexParam.anchorX = 0; thisTexParam.anchorY = 0; break;
                    case tk2dSpriteCollectionDefinition.Anchor.UpperCenter: thisTexParam.anchorX = texWidthI / 2; thisTexParam.anchorY = 0; break;
                    case tk2dSpriteCollectionDefinition.Anchor.UpperRight: thisTexParam.anchorX = texWidthI; thisTexParam.anchorY = 0; break;
                }
                Vector3 pos0 = new Vector3(-thisTexParam.anchorX * thisTexParam.scale.x * scale, 0, -(h - thisTexParam.anchorY * thisTexParam.scale.y) * scale);
				
				colliderOrigin = new Vector3(pos0.x, pos0.z, 0.0f);
                Vector3 pos1 = pos0 + new Vector3(scaleX, 0, scaleY);
				
				untrimmedPos0 = new Vector3(pos0.x, pos0.z);
				untrimmedPos1 = new Vector3(pos1.x, pos1.z);

				List<Vector3> positions = new List<Vector3>();
				List<Vector2> uvs = new List<Vector2>();

				// build mesh
				if (_lut.isSplit)
				{
					coll.spriteDefinitions[i].flipped = false; // each split could be rotated, but not consistently
					
					for (int j = 0; j < spriteLuts.Count; ++j)
					{
						if (spriteLuts[j].source == i)
						{
							_lut = spriteLuts[j];

							int thisAtlasIndex = 0;
							foreach (var p in packers)
							{
								if ((atlasEntry = p.FindEntryWithIndex(_lut.atlasIndex)) != null)
								{
									packer = p;
									break;
								}
								++thisAtlasIndex;
							}

							if (thisAtlasIndex != atlasIndex)
							{
								// This is a serious problem, dicing is not supported when multi atlas output is selected
								Debug.Break();
							}

							fwidth = packer.width;
				    	    fheight = packer.height;

				            tx = atlasEntry.x + padAmount;
							ty = atlasEntry.y + padAmount;
							tw = atlasEntry.w - padAmount * 2;
							th = atlasEntry.h - padAmount * 2;

				            sd_y = packer.height - ty - th;
				            v0 = new Vector2(tx / fwidth + uvOffsetX, 1.0f - (sd_y + th) / fheight + uvOffsetY);
				            v1 = new Vector2((tx + tw) / fwidth - uvOffsetX, 1.0f - sd_y / fheight - uvOffsetY);

							float x0 = _lut.rx / texWidth;
							float y0 = _lut.ry / texHeight;
							float x1 = (_lut.rx + _lut.rw) / texWidth;
							float y1 = (_lut.ry + _lut.rh) / texHeight;

							Vector3 dpos0 = new Vector3(Mathf.Lerp(pos0.x, pos1.x, x0), 0.0f, Mathf.Lerp(pos0.z, pos1.z, y0));
							Vector3 dpos1 = new Vector3(Mathf.Lerp(pos0.x, pos1.x, x1), 0.0f, Mathf.Lerp(pos0.z, pos1.z, y1));

							positions.Add(new Vector3(dpos0.x, dpos0.z, 0));
							positions.Add(new Vector3(dpos1.x, dpos0.z, 0));
							positions.Add(new Vector3(dpos0.x, dpos1.z, 0));
							positions.Add(new Vector3(dpos1.x, dpos1.z, 0));

			                if (atlasEntry.flipped)
			                {
			                    uvs.Add(new Vector2(v0.x,v0.y));
			                    uvs.Add(new Vector2(v0.x,v1.y));
			                    uvs.Add(new Vector2(v1.x,v0.y));
			                    uvs.Add(new Vector2(v1.x,v1.y));
			                }
			                else
			                {
			                    uvs.Add(new Vector2(v0.x,v0.y));
			                    uvs.Add(new Vector2(v1.x,v0.y));
			                    uvs.Add(new Vector2(v0.x,v1.y));
			                    uvs.Add(new Vector2(v1.x,v1.y));
			                }
						}
					}
				}
				else if (thisTexParam.customSpriteGeometry)
				{
					coll.spriteDefinitions[i].flipped = atlasEntry.flipped;
					
					List<int> indices = new List<int>();
					foreach (var island in thisTexParam.geometryIslands)
					{
						int baseIndex = positions.Count;
						for (int x = 0; x < island.points.Length; ++x)
						{
							var v = island.points[x];
							Vector2 origin = new Vector2(pos0.x, pos0.z);
							positions.Add(new Vector2(v.x * thisTexParam.scale.x, (texHeight - v.y) * thisTexParam.scale.y) * scale + new Vector2(origin.x, origin.y));
							
				            tx = atlasEntry.x + padAmount;
							ty = atlasEntry.y + padAmount;
							tw = atlasEntry.w - padAmount * 2;
							th = atlasEntry.h - padAmount * 2;

				            //v0 = new Vector2(tx / fwidth + uvOffsetX, 1.0f - (sd_y + th) / fheight + uvOffsetY);
				            //v1 = new Vector2((tx + tw) / fwidth - uvOffsetX, 1.0f - sd_y / fheight - uvOffsetY);
							
							Vector2 uv = new Vector2();
							if (atlasEntry.flipped)
							{
								uv.x = (tx - _lut.ry + texHeight - v.y) / fwidth + uvOffsetX;
								uv.y = (ty - _lut.rx + v.x) / fheight + uvOffsetY;
							}
							else
							{
								uv.x = (tx - _lut.rx + v.x) / fwidth + uvOffsetX;
								uv.y = (ty - _lut.ry + texHeight - v.y) / fheight + uvOffsetY ;
							}
							
							uvs.Add(uv);
						}
						
						tk2dEditor.Triangulator triangulator = new tk2dEditor.Triangulator(island.points);
						int[] localIndices = triangulator.Triangulate();
						//for (int x = localIndices.Length - 1; x >= 0; --x)
						for (int x = 0; x < localIndices.Length; x += 3)
						{
							indices.Add(baseIndex + localIndices[x + 2]);
							indices.Add(baseIndex + localIndices[x + 1]);
							indices.Add(baseIndex + localIndices[x + 0]);
						}
					}

					coll.spriteDefinitions[i].indices = indices.ToArray();
				}
				else
				{
					coll.spriteDefinitions[i].flipped = atlasEntry.flipped;
					
					float x0 = _lut.rx / texWidth;
					float y0 = _lut.ry / texHeight;
					float x1 = (_lut.rx + _lut.rw) / texWidth;
					float y1 = (_lut.ry + _lut.rh) / texHeight;

					Vector3 dpos0 = new Vector3(Mathf.Lerp(pos0.x, pos1.x, x0), 0.0f, Mathf.Lerp(pos0.z, pos1.z, y0));
					Vector3 dpos1 = new Vector3(Mathf.Lerp(pos0.x, pos1.x, x1), 0.0f, Mathf.Lerp(pos0.z, pos1.z, y1));

					positions.Add(new Vector3(dpos0.x, dpos0.z, 0));
					positions.Add(new Vector3(dpos1.x, dpos0.z, 0));
					positions.Add(new Vector3(dpos0.x, dpos1.z, 0));
					positions.Add(new Vector3(dpos1.x, dpos1.z, 0));

	                if (atlasEntry.flipped)
	                {
	                    uvs.Add(new Vector2(v0.x,v0.y));
	                    uvs.Add(new Vector2(v0.x,v1.y));
	                    uvs.Add(new Vector2(v1.x,v0.y));
	                    uvs.Add(new Vector2(v1.x,v1.y));
	                }
	                else
	                {
	                    uvs.Add(new Vector2(v0.x,v0.y));
	                    uvs.Add(new Vector2(v1.x,v0.y));
	                    uvs.Add(new Vector2(v0.x,v1.y));
	                    uvs.Add(new Vector2(v1.x,v1.y));
	                }
				}

				// build sprite definition
				if (!thisTexParam.customSpriteGeometry)
				{
					coll.spriteDefinitions[i].indices = new int[ 6 * (positions.Count / 4) ];
					for (int j = 0; j < positions.Count / 4; ++j)
					{
		                coll.spriteDefinitions[i].indices[j * 6 + 0] = j * 4 + 0;
						coll.spriteDefinitions[i].indices[j * 6 + 1] = j * 4 + 3;
						coll.spriteDefinitions[i].indices[j * 6 + 2] = j * 4 + 1;
						coll.spriteDefinitions[i].indices[j * 6 + 3] = j * 4 + 2;
						coll.spriteDefinitions[i].indices[j * 6 + 4] = j * 4 + 3;
						coll.spriteDefinitions[i].indices[j * 6 + 5] = j * 4 + 0;
					}
					coll.spriteDefinitions[i].complexGeometry = false;
				}
				else
				{
					coll.spriteDefinitions[i].complexGeometry = true;
				}
				
				if (positions.Count > 0)
				{
					// http://forum.unity3d.com/threads/98781-Compute-mesh-inertia-tensor-failed-for-one-of-the-actor-Behaves-differently-in-3.4
					Vector3 p = positions[positions.Count - 1];
					p.z -= 0.001f;
					positions[positions.Count - 1] = p;
				}
			
				coll.spriteDefinitions[i].positions = new Vector3[positions.Count];
				coll.spriteDefinitions[i].uvs = new Vector2[uvs.Count];
				for (int j = 0; j < positions.Count; ++j)
				{
					coll.spriteDefinitions[i].positions[j] = positions[j];
					coll.spriteDefinitions[i].uvs[j] = uvs[j];
				}
				
				// empty out to a sensible default, which corresponds to what Unity does by default
				coll.spriteDefinitions[i].normals = new Vector3[0];
				coll.spriteDefinitions[i].tangents = new Vector4[0];
				
				// fill out tangents and normals
				if (gen.normalGenerationMode != tk2dSpriteCollection.NormalGenerationMode.None)
				{
					Mesh tmpMesh = new Mesh();
					tmpMesh.vertices = coll.spriteDefinitions[i].positions;
					tmpMesh.uv = coll.spriteDefinitions[i].uvs;
					tmpMesh.triangles = coll.spriteDefinitions[i].indices;
					
					tmpMesh.RecalculateNormals();
					
					coll.spriteDefinitions[i].normals = tmpMesh.normals;

					if (gen.normalGenerationMode != tk2dSpriteCollection.NormalGenerationMode.NormalsOnly)
					{
						coll.spriteDefinitions[i].tangents = tmpMesh.tangents;
					}
				}
			}
			
			// fixup in case something went wrong
			if (coll.allowMultipleAtlases)
			{
				coll.spriteDefinitions[i].material = gen.atlasMaterials[atlasIndex];
			}
			else
			{
				coll.spriteDefinitions[i].material = gen.altMaterials[thisTexParam.materialId];
				coll.spriteDefinitions[i].materialId = thisTexParam.materialId;
				
				if (coll.spriteDefinitions[i].material == null) // fall back gracefully in case something went wrong
				{
					coll.spriteDefinitions[i].material = gen.atlasMaterials[atlasIndex];
					coll.spriteDefinitions[i].materialId = 0;
				}
			}

            Vector3 boundsMin = new Vector3(1.0e32f, 1.0e32f, 1.0e32f);
            Vector3 boundsMax = new Vector3(-1.0e32f, -1.0e32f, -1.0e32f);
            foreach (Vector3 v in coll.spriteDefinitions[i].positions)
            {
                boundsMin = Vector3.Min(boundsMin, v);
                boundsMax = Vector3.Max(boundsMax, v);
            }
			
			coll.spriteDefinitions[i].boundsData = new Vector3[2];
			coll.spriteDefinitions[i].boundsData[0] = (boundsMax + boundsMin) / 2.0f;
			coll.spriteDefinitions[i].boundsData[1] = (boundsMax - boundsMin);

			// this is the dimension of exactly one pixel, scaled to match sprite dimensions and scale
			coll.spriteDefinitions[i].texelSize = new Vector3(scale * thisTexParam.scale.x, scale * thisTexParam.scale.y, 0.0f);
			
			coll.spriteDefinitions[i].untrimmedBoundsData = new Vector3[2];
			if (mesh)
			{
				// custom meshes aren't trimmed, the untrimmed bounds are exactly the same as the regular ones
				coll.spriteDefinitions[i].untrimmedBoundsData[0] = coll.spriteDefinitions[i].boundsData[0];
				coll.spriteDefinitions[i].untrimmedBoundsData[1] = coll.spriteDefinitions[i].boundsData[1];
			}
			else
			{
				boundsMin = Vector3.Min(untrimmedPos0, untrimmedPos1);
				boundsMax = Vector3.Max(untrimmedPos0, untrimmedPos1);
				coll.spriteDefinitions[i].untrimmedBoundsData[0] = (boundsMax + boundsMin) / 2.0f;
				coll.spriteDefinitions[i].untrimmedBoundsData[1] = (boundsMax - boundsMin);
			}
			
			
			coll.spriteDefinitions[i].name = gen.textureParams[i].name;

			// Generate collider data here
			UpdateColliderData(gen, scale, coll, i, colliderOrigin);
        }
    }
	
	static void UpdateColliderData(tk2dSpriteCollection gen, float scale, tk2dSpriteCollectionData coll, int spriteIndex, Vector3 origin)
	{
		var colliderType = gen.textureParams[spriteIndex].colliderType;
		var def = coll.spriteDefinitions[spriteIndex];
		var src = gen.textureParams[spriteIndex];
		
		def.colliderVertices = null;
		def.colliderIndicesFwd = null;
		def.colliderIndicesBack = null;
		
		float texHeight = 0;
		if (src.extractRegion)
		{
			texHeight = src.regionH;
		}
		else
		{
			texHeight = gen.textureRefs[spriteIndex]?gen.textureRefs[spriteIndex].height:2.0f;
		}
		
		if (colliderType == tk2dSpriteCollectionDefinition.ColliderType.BoxTrimmed)
		{
			def.colliderVertices = new Vector3[2];
			def.colliderVertices[0] = def.boundsData[0];
			def.colliderVertices[1] = def.boundsData[1] * 0.5f; // extents is 1/2x size
			def.colliderVertices[1].z = gen.physicsDepth;
			def.colliderType = tk2dSpriteDefinition.ColliderType.Box;
		}
		else if (colliderType == tk2dSpriteCollectionDefinition.ColliderType.BoxCustom)
		{
			Vector2 v0 = new Vector3(src.boxColliderMin.x * src.scale.x, (texHeight - src.boxColliderMax.y) * src.scale.y) * scale + origin;
			Vector2 v1 = new Vector3(src.boxColliderMax.x * src.scale.x, (texHeight - src.boxColliderMin.y) * src.scale.y) * scale + origin;
			
			def.colliderVertices = new Vector3[2];
			def.colliderVertices[0] = (v0 + v1) * 0.5f;
			def.colliderVertices[1] = (v1 - v0) * 0.5f;
			def.colliderVertices[1].z = gen.physicsDepth;
			def.colliderType = tk2dSpriteDefinition.ColliderType.Box;
		}
		else if (colliderType == tk2dSpriteCollectionDefinition.ColliderType.Polygon)
		{
			List<Vector3> meshVertices = new List<Vector3>();
			List<int> meshIndicesFwd = new List<int>();
			
			foreach (var island in src.polyColliderIslands)
			{
				List<Vector2> points = new List<Vector2>();
				List<Vector2> points2D = new List<Vector2>();
			
				
				// List all points
				for (int i = 0; i < island.points.Length; ++i)
				{
					Vector2 v = island.points[i];
					points.Add(new Vector2(v.x * src.scale.x, (texHeight - v.y) * src.scale.y) * scale + new Vector2(origin.x, origin.y));
				}
				
				int baseIndex = meshVertices.Count;
				for (int i = 0; i < points.Count; ++i)
				{
					meshVertices.Add( new Vector3(points[i].x, points[i].y, -gen.physicsDepth) );
					meshVertices.Add( new Vector3(points[i].x, points[i].y,  gen.physicsDepth) );
					points2D.Add( new Vector2(points[i].x, points[i].y) );
				}
				
				// body
				int numPoints = island.connected?points.Count:(points.Count - 1);
				for (int i = 0; i < numPoints; ++i)
				{
					int i0 = i * 2;
					int i1 = i0 + 1;
					int i2 = ((i + 1)%island.points.Length) * 2;
					int i3 = i2 + 1;
					
					meshIndicesFwd.Add(baseIndex + i2);
					meshIndicesFwd.Add(baseIndex + i1);
					meshIndicesFwd.Add(baseIndex + i0);
					meshIndicesFwd.Add(baseIndex + i2);
					meshIndicesFwd.Add(baseIndex + i3);
					meshIndicesFwd.Add(baseIndex + i1);
				}

				// cap if allowed and necessary
				var cap = src.polyColliderCap;
				if (island.connected && cap != tk2dSpriteCollectionDefinition.PolygonColliderCap.None)
				{
					tk2dEditor.Triangulator triangulator = new tk2dEditor.Triangulator(points2D.ToArray());
					int[] indices = triangulator.Triangulate();
					
					if (cap == tk2dSpriteCollectionDefinition.PolygonColliderCap.Front || cap == tk2dSpriteCollectionDefinition.PolygonColliderCap.FrontAndBack)
					{
						for (int i = 0; i < indices.Length; ++i)
							meshIndicesFwd.Add(baseIndex + indices[i] * 2);
					}
					
					if (cap == tk2dSpriteCollectionDefinition.PolygonColliderCap.Back || cap == tk2dSpriteCollectionDefinition.PolygonColliderCap.FrontAndBack)
					{
						for (int i = 0; i < indices.Length; i += 3)
						{
							meshIndicesFwd.Add(baseIndex + indices[i + 2] * 2 + 1);
							meshIndicesFwd.Add(baseIndex + indices[i + 1] * 2 + 1);
							meshIndicesFwd.Add(baseIndex + indices[i + 0] * 2 + 1);
						}
					}
				}
			}
			
			int[] meshIndicesBack = new int[meshIndicesFwd.Count];
			for (int i = 0; i < meshIndicesFwd.Count; i += 3)
			{
				meshIndicesBack[i + 0] = meshIndicesFwd[i + 2];
				meshIndicesBack[i + 1] = meshIndicesFwd[i + 1];
				meshIndicesBack[i + 2] = meshIndicesFwd[i + 0];
			}
			
			def.colliderVertices = meshVertices.ToArray();
			def.colliderIndicesFwd = meshIndicesFwd.ToArray();
			def.colliderIndicesBack = meshIndicesBack;
			def.colliderConvex = src.colliderConvex;
			def.colliderType = tk2dSpriteDefinition.ColliderType.Mesh;
			def.colliderSmoothSphereCollisions = src.colliderSmoothSphereCollisions;
		}
		else if (colliderType == tk2dSpriteCollectionDefinition.ColliderType.None)
		{
			def.colliderType = tk2dSpriteDefinition.ColliderType.None;
		}
		else if (colliderType == tk2dSpriteCollectionDefinition.ColliderType.Unset)
		{
			def.colliderType = tk2dSpriteDefinition.ColliderType.Unset;
		}
	}
}
