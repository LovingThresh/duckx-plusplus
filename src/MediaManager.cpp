/*!
 * @file MediaManager.cpp
 * @brief Implementation of media element management system
 * 
 * Handles embedding of media files (images, textboxes) including
 * file operations, relationship management, and XML generation.
 */

#include "MediaManager.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "DocxFile.hpp"
#include "Image.hpp"

#include "BaseElement.hpp"
#include "Document.hpp"
#include "TextBox.hpp"

namespace duckx
{
    unsigned int safe_stoui(const char* str)
    {
        if (!str) return 0;

        const std::string s(str);
        if (s.empty() || s.find_first_not_of("0123456789") != std::string::npos)
        {
            return 0; // Not a valid positive number
        }

        unsigned int val;
        std::stringstream ss(s);
        ss >> val;

        if (ss.fail() || !ss.eof())
        {
            return 0; // Conversion failed or had extra character
        }

        return val;
    }

    MediaManager::MediaManager(Document* owner_doc, DocxFile* file, pugi::xml_document* rels_xml, pugi::xml_document* doc_xml,
                               pugi::xml_document* content_types_xml)
        : m_file(file), m_rels_xml(rels_xml), m_doc_xml(doc_xml), m_content_types_xml(content_types_xml), m_doc(owner_doc)
    {
        if (!m_doc || !m_file || !m_rels_xml || !m_doc_xml)
        {
            throw std::logic_error("MediaManager requires valid DocxFile, rels XML, and document XML.");
        }

        unsigned int max_docpr_id = 0;
        const pugi::xpath_query docpr_query("//wp:docPr[@id] | //wps:docPr[@id]");
        const pugi::xpath_node_set results = m_doc_xml->select_nodes(docpr_query);

        for (pugi::xpath_node xpath_n: results)
        {
            pugi::xml_node docpr_node = xpath_n.node();
            const unsigned int current_id = safe_stoui(docpr_node.attribute("id").value());
            if (current_id > max_docpr_id)
            {
                max_docpr_id = current_id;
            }
        }
        m_docpr_id_counter = max_docpr_id + 1;
    }

    Run MediaManager::add_image(const Paragraph& p, const Image& image)
    {
        // 1. Add the physical file to the zip and create the relationship
        const std::string media_target = add_media_to_zip(image.get_path());
        const std::string rId = add_image_relationship(media_target);
        const unsigned int drawing_id = m_doc->get_unique_rid();

        // 2. Get the paragraph's XML node
        pugi::xml_node p_node = p.get_node();
        if (!p_node)
        {
            throw std::runtime_error("Cannot add image to an invalid paragraph.");
        }

        // 3. Create the XML structure for the image
        pugi::xml_node new_run_node = p_node.append_child("w:r");
        image.generate_drawing_xml(new_run_node, rId, drawing_id);

        // 4. Return a lightweight Run object
        return {p_node, new_run_node};
    }

    Run MediaManager::add_textbox(const Paragraph& p, const TextBox& textbox)
    {
        pugi::xml_node paragraph_node = p.get_node();
        if (!paragraph_node)
        {
            return {};
        }

        pugi::xml_node run_node = paragraph_node.append_child("w:r");
        const unsigned int drawing_id = get_unique_docpr_id();
        textbox.generate_drawing_xml(run_node, "", drawing_id);
        return {paragraph_node, run_node};
    }

    // Private helper implementations
    std::string MediaManager::add_media_to_zip(const std::string& file_path)
    {
        // 1. 读取图片文件的二进制内容
        std::ifstream ifs(file_path, std::ios::binary);
        if (!ifs)
        {
            throw std::runtime_error("Cannot open image file: " + file_path);
        }
        const std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        // 2. 提取文件扩展名并转换为小写
        size_t dot_pos = file_path.find_last_of('.');
        if (dot_pos == std::string::npos)
        {
            throw std::runtime_error("File has no extension: " + file_path);
        }
        const std::string ext_with_dot = file_path.substr(dot_pos);
        std::string ext_lower = ext_with_dot.substr(1);
        std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(),
                       [](const unsigned char c) { return std::tolower(c); });

        // 3. 检查并按需更新 [Content_Types].xml
        // 静态映射表，用于查找扩展名对应的标准内容类型
        static const std::map<std::string, std::string> content_type_map = {
                {"png", "image/png"}, {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},
                {"gif", "image/gif"}, {"bmp", "image/bmp"},  {"tiff", "image/tiff"}};

        auto it = content_type_map.find(ext_lower);
        if (it == content_type_map.end())
        {
            throw std::runtime_error("Unsupported image extension: " + ext_lower);
        }
        const std::string& content_type = it->second;

        // 确保 m_content_types_xml 指针有效
        if (!m_content_types_xml)
        {
            throw std::logic_error("Content types XML is not loaded in MediaManager.");
        }

        pugi::xml_node types_root = m_content_types_xml->child("Types");
        if (types_root)
        {
            // 使用 XPath 查询是否已存在该扩展名的 <Default> 标签
            std::string query = "/Types/Default[@Extension='" + ext_lower + "']";
            if (!types_root.select_node(query.c_str()))
            {
                // 如果不存在，则添加一个新的 <Default> 标签
                pugi::xml_node new_default = types_root.append_child("Default");
                new_default.append_attribute("Extension").set_value(ext_lower.c_str());
                new_default.append_attribute("ContentType").set_value(content_type.c_str());
            }
        }

        // 4. 将图片文件写入 ZIP 包
        const std::string internal_path = "word/media/image" + std::to_string(m_media_id_counter++) + ext_with_dot;
        m_file->write_entry(internal_path, content);

        // 5. 返回在 .rels 文件中需要使用的相对路径
        return "media/" + internal_path.substr(internal_path.find_last_of('/') + 1);
    }

    std::string MediaManager::add_image_relationship(const std::string& media_target) const
    {
        const auto IMAGE_REL_TYPE = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image";
        std::string new_rid = m_doc->get_next_relationship_id();

        pugi::xml_node relationships = m_rels_xml->child("Relationships");
        if (!relationships)
        { // If .rels was empty or malformed
            relationships = m_rels_xml->append_child("Relationships");
            relationships.append_attribute("xmlns") = "http://schemas.openxmlformats.org/package/2006/relationships";
        }
        pugi::xml_node new_rel = relationships.append_child("Relationship");
        new_rel.append_attribute("Id") = new_rid.c_str();
        new_rel.append_attribute("Type") = IMAGE_REL_TYPE;
        new_rel.append_attribute("Target") = media_target.c_str();

        return new_rid;
    }

    unsigned int MediaManager::get_unique_docpr_id()
    {
        return m_docpr_id_counter++;
    }
} // namespace duckx
