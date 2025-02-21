﻿
#include <stdio.h>
#include <cstdlib>

#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_matrix.h"
#include "ssplayer_render.h"
#include "sscharconverter.h"

#include "../Helper/DebugPrint.h"

#include <memory>
#include <utility>


namespace spritestudio6
{

bool SsCellMapList::preloadTexture(SsProject* proj)
{

	for (auto i = proj->textureList.begin(); i != proj->textureList.end(); i++)
	{
		SsString fname = (*i);
		SSTextureFactory::loadTexture(fname.c_str());
	}

	return true;
}

bool SsCellMapList::unloadTexture(SsProject* proj)
{
	for (auto i = proj->textureList.begin(); i != proj->textureList.end(); i++)
	{
		SsString fname = (*i);
		SSTextureFactory::releaseTextureForced(fname.c_str());
	}

	//SSTextureFactory::releaseAllTexture();

	return true;
}


SsCelMapLinker::SsCelMapLinker(SsCellMap* cellmap, SsString filePath)
{

	cellMap = cellmap;
	size_t num = cellMap->cells.size();
	for (size_t i = 0; i < num; i++)
	{
		CellDic[cellMap->cells[i]->name] = cellMap->cells[i];
	}

	if (!SSTextureFactory::isExist())
	{
		puts("SSTextureFactory not created yet.");
#ifndef _NOTUSE_EXCEPTION		
		throw;
#endif		
	}
#ifdef _NOTUSE_TEXTURE_FULLPATH
	//スルー
	SsString filePathFs = cellmap->imagePath;
#else
	std::string fullpath = getFullPath(filePath, path2dir(cellmap->imagePath));
	fullpath = fullpath + path2file(cellmap->imagePath);
	fullpath = nomarizeFilename(fullpath);

//	DEBUG_PRINTF("TextureFile Load %s \n", fullpath.c_str());
//	tex = SSTextureFactory::loadTexture(fullpath.c_str());
	SsString filePathFs = SsCharConverter::convert_path_string( fullpath );
#endif
	DEBUG_PRINTF("TextureFile Load %s \n", filePathFs.c_str());
	tex = SSTextureFactory::loadTexture(filePathFs.c_str());

}



void	SsCellMapList::clear()
{
	if (CellMapDic.size() > 0)
	{
		for (CellMapDicItr itr = CellMapDic.begin(); itr != CellMapDic.end();)
		{
			if (itr->second)
			{
				itr->second.reset();
				continue;
			}
			itr++;
		}
	}

	CellMapDic.clear();
	CellMapList.clear();
}


void	SsCellMapList::setCellMapPath(  const SsString& filepath )
{
	CellMapPath = filepath;
}

void	SsCellMapList::set(SsProject* proj , SsAnimePack* animepack )
{
	clear();
	setCellMapPath( proj->getImageBasepath() );

	for ( size_t i = 0 ; i < animepack->cellmapNames.size() ; i++ )
	{
		SsCellMap* cell = proj->findCellMap( animepack->cellmapNames[i] );
		if ( cell==0 )
		{
			DEBUG_PRINTF( " Not found cellmap = %s" , animepack->cellmapNames[i].c_str() );
		}else{
			addIndex( cell );
		}
	}

	for ( size_t i = 0 ; i < proj->cellmapNames.size() ; i++ )
	{
		SsCellMap* cell = proj->findCellMap( proj->cellmapNames[i] );
		if ( cell==0 )
		{
			DEBUG_PRINTF( " Not found cellmap = %s" , animepack->cellmapNames[i].c_str() );
		}else{
			addMap( cell );
		}
	}


}
void	SsCellMapList::addMap(SsCellMap* cellmap)
{
	CellMapDic[ cellmap->name+".ssce" ].reset( new SsCelMapLinker(cellmap , this->CellMapPath ) );
}

void	SsCellMapList::addIndex(SsCellMap* cellmap)
{
	std::unique_ptr<SsCelMapLinker> cellmapLinker( new SsCelMapLinker(cellmap , this->CellMapPath ) );
	CellMapList.push_back( std::move( cellmapLinker ) );
}

SsCelMapLinker*	SsCellMapList::getCellMapLink( const SsString& name )
{
	CellMapDicItr itr = CellMapDic.find(name);
	if ( itr != CellMapDic.end() )
	{
		return itr->second.get();
	}else{
		for ( itr=CellMapDic.begin() ; itr != CellMapDic.end() ; itr++)
		{
			if ( (itr->second.get())->cellMap->loadFilepath == name )
			{
				return itr->second.get();
			}
		}
	}

	return 0;
}


void getCellValue( SsCelMapLinker* l, SsString& cellName , SsCellValue& v )
{
	v.cell = l->findCell( cellName );

	v.filterMode = l->cellMap->filterMode;
	v.wrapMode = l->cellMap->wrapMode;

	if ( l->tex )
	{
		v.texture = l->tex;
	}
	else
	{
		v.texture = 0;
	}

	calcUvs( &v );
}

void getCellValue( SsCellMapList* cellList, SsString& cellMapName , SsString& cellName , SsCellValue& v )
{
	SsCelMapLinker* l = cellList->getCellMapLink( cellMapName );
	getCellValue( l , cellName , v );


}

void getCellValue( SsCellMapList* cellList, int cellMapid , SsString& cellName , SsCellValue& v )
{
	SsCelMapLinker* l = cellList->getCellMapLink( cellMapid );
	if (l)
	{
		getCellValue(l, cellName, v);
	}


}

void calcUvs( SsCellValue* cellv )
{
	//SsCellMap* map = cellv->cellmapl->cellMap;
	SsCell* cell = cellv->cell;
	if ( cellv->texture == 0 ) return ;

//	if ( cell == 0 || map == 0)
	if ( cell == 0 )
//	if ( ( cell == 0 ) || ( cellv->texture == 0 ) )	//koizumi change
	{
		cellv->uvs[0].x = cellv->uvs[0].y = 0;
		cellv->uvs[1].x = cellv->uvs[1].y = 0;
		cellv->uvs[2].x = cellv->uvs[2].y = 0;
		cellv->uvs[3].x = cellv->uvs[3].y = 0;
		return;
	}

	SsVector2 wh;
	wh.x = (float)cellv->texture->getWidth();
	wh.y = (float)cellv->texture->getHeight();

//	SsVector2 wh = map->pixelSize;
	// 右上に向かって＋になる
	float left = cell->pos.x / wh.x;
	float right = (cell->pos.x + cell->size.x) / wh.x;


	// LB->RB->LT->RT 順
	// 頂点をZ順にしている都合上UV値は上下逆転させている
	float top = cell->pos.y / wh.y;
	float bottom = ( cell->pos.y + cell->size.y) / wh.y;

	if (cell->rotated)
	{
		// 反時計回りに９０度回転されているため起こして描画されるようにしてやる。
		// 13
		// 02
		cellv->uvs[0].x = cellv->uvs[1].x = left;
		cellv->uvs[2].x = cellv->uvs[3].x = right;
		cellv->uvs[1].y = cellv->uvs[3].y = top;
		cellv->uvs[0].y = cellv->uvs[2].y = bottom;
	}
	else
	{
		// そのまま。頂点の順番は下記の通り
		// 01
		// 23
		cellv->uvs[0].x = cellv->uvs[2].x = left;
		cellv->uvs[1].x = cellv->uvs[3].x = right;
		cellv->uvs[0].y = cellv->uvs[1].y = top;
		cellv->uvs[2].y = cellv->uvs[3].y = bottom;
	}
}

}	// namespace spritestudio6

