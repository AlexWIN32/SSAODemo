/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Font.h>
#include <CommonParams.h>
#include <Xml.h>
#include <Texture.h>
#include <Utils.h>

namespace GUI
{

void Font::Init(const std::string &FontDescriptionFilePath) throw (Exception)
{
    XML::XmlData xmlFile;
    xmlFile.LoadFromFile("../Resources/Fonts/" + FontDescriptionFilePath);
    
    const XML::Node &fontNode = xmlFile.GetRoot();
    const XML::Node &commonNode = fontNode.GetNode("common");

    name = fontNode.GetNode("info").GetProperty("face");

    lineHeight = atoi(commonNode.GetProperty("lineHeight").c_str());
    lineScreenHeight = (float)lineHeight / CommonParams::GetScreenHeight();

    FLOAT texWidth = atof(commonNode.GetProperty("scaleW").c_str());
    FLOAT texHeight = atof(commonNode.GetProperty("scaleH").c_str());

    INT pagesCnt = atoi(commonNode.GetProperty("pages").c_str());
    if(pagesCnt != 1)
        throw FontException("only one page in font supported");

    const std::string &texFileName = fontNode.GetNode("pages").GetNode("page").GetProperty("file");

    texture = Texture::LoadTexture2DFromFile("../Resources/Textures/" + texFileName);

    const XML::Node &chars = xmlFile.GetRoot().GetNode("chars");

    size_t charsCnt = chars.GetNodesCount("char");

    for(size_t i = 0; i < charsCnt; i++){
        const XML::Node &charNode = chars.GetNode("char", i);

        wchar_t ch = (wchar_t)atoi(charNode.GetProperty("id").c_str());

        INT x = atoi(charNode.GetProperty("x").c_str());
        INT y = atoi(charNode.GetProperty("y").c_str());

        INT width = atoi(charNode.GetProperty("width").c_str());
        INT height = atoi(charNode.GetProperty("height").c_str());

        INT offsetX = atoi(charNode.GetProperty("xoffset").c_str());
        INT offsetY = atoi(charNode.GetProperty("yoffset").c_str());

        INT xAnvance = atoi(charNode.GetProperty("xadvance").c_str());
        
        Glyph newGlyph;
        newGlyph.tcTopLeft.x = (FLOAT)x / texWidth;
        newGlyph.tcTopLeft.y = (FLOAT)y / texHeight;
        newGlyph.tcBottomRight.x = (FLOAT)(x + width) / texWidth;
        newGlyph.tcBottomRight.y = (FLOAT)(y + height) / texHeight;
        newGlyph.offset.x = offsetX;
        newGlyph.offset.y = offsetY;
        newGlyph.width = width;
        newGlyph.height = height;
        newGlyph.xAdvance = xAnvance;

        glyphs.insert(std::make_pair(ch, newGlyph));
    }

    const XML::Node &kerningsNode = xmlFile.GetRoot().GetNode("kernings");

    size_t kerningsCnt = kerningsNode.GetNodesCount("kerning");

    for(size_t k = 0; k < kerningsCnt; k++){
        const XML::Node &kerningNode = kerningsNode.GetNode("kerning", k);
    
        wchar_t first = (wchar_t)atoi(kerningNode.GetProperty("first").c_str());
        wchar_t second = (wchar_t)atoi(kerningNode.GetProperty("second").c_str());

        INT amount = atoi(kerningNode.GetProperty("amount").c_str());

        kernings.insert(std::make_pair(Kerning(first, second), amount));
    }
}

void Font::Construct(const Font &Var)
{
    kernings = Var.kernings;
    glyphs = Var.glyphs;
    name = Var.name;
    lineHeight = Var.lineHeight;
    lineScreenHeight = Var.lineScreenHeight;

    if(texture)
        texture->Release();

    if(Var.texture){
        Var.texture->AddRef();
        texture = Var.texture;
    }
}

const Font::Glyph &Font::operator[] (wchar_t Char) const throw (Exception)
{
    GlyphDescriptionsStorage::const_iterator ci = glyphs.find(Char);
    if(ci == glyphs.end())
        throw FontException("char " + Utils::to_string((int)Char) + " not found in font "+ name);

    return ci->second;
}

bool Font::FindKerning(const Kerning &KerningData, INT &Offset) const
{
    KerningsStorage::const_iterator ci = kernings.find(KerningData);
    if(ci == kernings.end())
        return false;

    Offset = ci->second;
    return true;
}

D3DXVECTOR2 Font::MeasureString(const std::wstring &String) const throw (Exception)
{
    D3DXVECTOR2 size(0.0f, 0.0f);
    float rowXOffset = 0.0f;

    std::wstring::const_iterator ci;
    for(ci = String.begin(); ci != String.end(); ++ci){
        wchar_t ch = *ci;
        if(ch == '\n'){
            
            if(size.x < rowXOffset)
                size.x = rowXOffset;

            size.y += GetLineHeight();

            rowXOffset = 0.0f;
        }else{
            const Font::Glyph &glyph = operator[](ch);

            rowXOffset += Control::WigthToX(glyph.xAdvance);

            std::wstring::const_iterator next = ci + 1;
            if(next != String.end()){
                INT offset = 0;
                if(FindKerning(Font::Kerning(ch, *next), offset))
                     rowXOffset += Control::WigthToX(offset);
            }
        }
    }

    if(rowXOffset != 0){
        size.y += GetLineHeight();
        if(size.x < rowXOffset)
            size.x = rowXOffset;
    }

    size.x = Control::XToWigth(size.x / CommonParams::GetScreenWidth());
    size.y = size.y / CommonParams::GetScreenHeight();

    return size;
}

}