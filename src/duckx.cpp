#include "duckx.hpp"

#include <cctype>
#include <ctime>
#include <iostream>
#include <utility>
#include <zip.h>

// Hack on pugixml
// We need to write xml to std string (or char *)
// So overload the write function
struct xml_string_writer : pugi::xml_writer
{
    std::string result;

    // ReSharper disable once CppParameterMayBeConst
    void write(const void* data, size_t size) override { result.append(static_cast<const char*>(data), size); }
};

duckx::Run::Run() = default;

duckx::Run::Run(const pugi::xml_node parent, const pugi::xml_node current)
{
    this->set_parent(parent);
    this->set_current(current);
}

void duckx::Run::set_parent(const pugi::xml_node node)
{
    this->parent = node;
    this->current = this->parent.child("w:r");
}

void duckx::Run::set_current(const pugi::xml_node node)
{
    this->current = node;
}

std::string duckx::Run::get_text() const
{
    return this->current.child("w:t").text().get();
}

bool duckx::Run::set_text(const std::string& text) const
{
    return this->current.child("w:t").text().set(text.c_str());
}

bool duckx::Run::set_text(const char* text) const
{
    return this->current.child("w:t").text().set(text);
}

duckx::Run& duckx::Run::next()
{
    this->current = this->current.next_sibling();
    return *this;
}

bool duckx::Run::has_next() const { return this->current != nullptr; }

// Table cells
duckx::TableCell::TableCell() = default;

duckx::TableCell::TableCell(const pugi::xml_node parent, const pugi::xml_node current)
{
    this->set_parent(parent);
    this->set_current(current);
}

void duckx::TableCell::set_parent(const pugi::xml_node node)
{
    this->parent = node;
    this->current = this->parent.child("w:tc");

    this->paragraph.set_parent(this->current);
}

void duckx::TableCell::set_current(const pugi::xml_node node)
{
    this->current = node;
}

bool duckx::TableCell::has_next() const
{
    return this->current != nullptr;
}

duckx::TableCell& duckx::TableCell::next()
{
    this->current = this->current.next_sibling();
    return *this;
}

duckx::Paragraph& duckx::TableCell::paragraphs()
{
    this->paragraph.set_parent(this->current);
    return this->paragraph;
}

// Table rows
duckx::TableRow::TableRow() = default;

duckx::TableRow::TableRow(const pugi::xml_node parent, const pugi::xml_node current)
{
    this->set_parent(parent);
    this->set_current(current);
}

void duckx::TableRow::set_parent(const pugi::xml_node node)
{
    this->parent = node;
    this->current = this->parent.child("w:tr");

    this->cell.set_parent(this->current);
}

void duckx::TableRow::set_current(const pugi::xml_node node) { this->current = node; }

duckx::TableRow& duckx::TableRow::next()
{
    this->current = this->current.next_sibling();
    return *this;
}

duckx::TableCell& duckx::TableRow::cells()
{
    this->cell.set_parent(this->current);
    return this->cell;
}

bool duckx::TableRow::has_next() const { return this->current != nullptr; }

// Tables
duckx::Table::Table() = default;

duckx::Table::Table(const pugi::xml_node parent, const pugi::xml_node current)
{
    this->set_parent(parent);
    this->set_current(current);
}

void duckx::Table::set_parent(const pugi::xml_node node)
{
    this->parent = node;
    this->current = this->parent.child("w:tbl");

    this->row.set_parent(this->current);
}

bool duckx::Table::has_next() const { return this->current != nullptr; }

duckx::Table& duckx::Table::next()
{
    this->current = this->current.next_sibling();
    this->row.set_parent(this->current);
    return *this;
}

void duckx::Table::set_current(const pugi::xml_node node) { this->current = node; }

duckx::TableRow& duckx::Table::rows()
{
    this->row.set_parent(this->current);
    return this->row;
}

duckx::Paragraph::Paragraph() = default;

duckx::Paragraph::Paragraph(const pugi::xml_node parent, const pugi::xml_node current)
{
    this->set_parent(parent);
    this->set_current(current);
}

void duckx::Paragraph::set_parent(const pugi::xml_node node)
{
    this->parent = node;
    this->current = this->parent.child("w:p");

    this->run.set_parent(this->current);
}

void duckx::Paragraph::set_current(const pugi::xml_node node) { this->current = node; }

duckx::Paragraph& duckx::Paragraph::next()
{
    this->current = this->current.next_sibling();
    this->run.set_parent(this->current);
    return *this;
}

bool duckx::Paragraph::has_next() const { return this->current != nullptr; }

duckx::Run& duckx::Paragraph::runs()
{
    this->run.set_parent(this->current);
    return this->run;
}

duckx::Run& duckx::Paragraph::add_run(const std::string& text, duckx::formatting_flag f)
{
    return this->add_run(text.c_str(), f);
}

duckx::Run& duckx::Paragraph::add_run(const char* text, duckx::formatting_flag f)
{
    // Add new run
    pugi::xml_node new_run = this->current.append_child("w:r");
    // Insert meta to new run
    pugi::xml_node meta = new_run.append_child("w:rPr");

    if (f & duckx::bold)
        meta.append_child("w:b");

    if (f & duckx::italic)
        meta.append_child("w:i");

    if (f & duckx::underline)
        meta.append_child("w:u").append_attribute("w:val").set_value("single");

    if (f & duckx::strikethrough)
        meta.append_child("w:strike").append_attribute("w:val").set_value("true");

    if (f & duckx::superscript)
        meta.append_child("w:vertAlign").append_attribute("w:val").set_value("superscript");
    else if (f & duckx::subscript)
        meta.append_child("w:vertAlign").append_attribute("w:val").set_value("subscript");

    if (f & duckx::smallcaps)
        meta.append_child("w:smallCaps").append_attribute("w:val").set_value("true");

    if (f & duckx::shadow)
        meta.append_child("w:shadow").append_attribute("w:val").set_value("true");

    pugi::xml_node new_run_text = new_run.append_child("w:t");
    // If the run starts or ends with whitespace characters, preserve them using
    // the xml:space attribute
    if (*text != 0 && (isspace(text[0]) || isspace(text[strlen(text) - 1])))
        new_run_text.append_attribute("xml:space").set_value("preserve");
    new_run_text.text().set(text);

    return *new Run(this->current, new_run);
}

duckx::Paragraph& duckx::Paragraph::insert_paragraph_after(const std::string& text, duckx::formatting_flag f)
{
    const pugi::xml_node new_para = this->parent.insert_child_after("w:p", this->current);

    const auto p = new Paragraph();
    p->set_current(new_para);
    p->add_run(text, f);

    return *p;
}

duckx::Document::Document()
{
    // TODO: this function must be removed!
    this->directory = "";
}

duckx::Document::Document(std::string directory)
{
    this->directory = std::move(directory);
}

bool duckx::Document::create()
{
    if (this->directory.empty())
    {
        return false;
    }
    return this->create(this->directory);
}

bool duckx::Document::create(const std::string& filename)
{
    this->directory = filename;

    // 创建新的zip文件
    zip_t* zip = zip_open(filename.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    if (!zip)
    {
        this->flag_is_open = false;
        return false;
    }

    // 创建docx的基本结构
    create_basic_structure(zip);

    // 关闭zip文件
    zip_close(zip);

    // 设置空的document结构
    this->document.load_string(get_empty_document_xml().c_str());
    this->flag_is_open = true;

    // 初始化paragraph
    this->paragraph.set_parent(document.child("w:document").child("w:body"));

    return true;
}

void duckx::Document::create_basic_structure(zip_t* zip)
{
    // 1. [Content_Types].xml
    zip_entry_open(zip, "[Content_Types].xml");
    const std::string content_types = get_content_types_xml();
    zip_entry_write(zip, content_types.c_str(), content_types.length());
    zip_entry_close(zip);

    // 2. _rels/.rels
    zip_entry_open(zip, "_rels/.rels");
    const std::string rels = get_rels_xml();
    zip_entry_write(zip, rels.c_str(), rels.length());
    zip_entry_close(zip);

    // 3. docProps/app.xml
    zip_entry_open(zip, "docProps/app.xml");
    const std::string app = get_app_xml();
    zip_entry_write(zip, app.c_str(), app.length());
    zip_entry_close(zip);

    // 4. docProps/core.xml
    zip_entry_open(zip, "docProps/core.xml");
    const std::string core = get_core_xml();
    zip_entry_write(zip, core.c_str(), core.length());
    zip_entry_close(zip);

    // 5. word/_rels/document.xml.rels
    zip_entry_open(zip, "word/_rels/document.xml.rels");
    const std::string doc_rels = get_document_rels_xml();
    zip_entry_write(zip, doc_rels.c_str(), doc_rels.length());
    zip_entry_close(zip);

    // 6. word/document.xml
    zip_entry_open(zip, "word/document.xml");
    const std::string document_xml = get_empty_document_xml();
    zip_entry_write(zip, document_xml.c_str(), document_xml.length());
    zip_entry_close(zip);
}

std::string duckx::Document::get_content_types_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
    <Default Extension="xml" ContentType="application/xml"/>
    <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
    <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
    <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
</Types>)";
}

std::string duckx::Document::get_rels_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
    <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
    <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>)";
}

std::string duckx::Document::get_app_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
    <Application>DuckX</Application>
    <DocSecurity>0</DocSecurity>
    <ScaleCrop>false</ScaleCrop>
    <SharedDoc>false</SharedDoc>
    <HyperlinksChanged>false</HyperlinksChanged>
    <AppVersion>1.0</AppVersion>
</Properties>)";
}

std::string duckx::Document::get_core_xml()
{
    // 获取当前时间
    const auto now = std::time(nullptr);
    tm tm_utc{};

#if defined(_MSC_VER) // 通常 gmtime_s 在MSVC下可用
    const errno_t err = gmtime_s(&tm_utc, &now);
    if (err == 0)
    {
    }
    else
    {
        std::cerr << "Error: gmtime_s failed with code " << err << std::endl;
    }
#else
    std::cerr << "gmtime_s is not available or not checked for this compiler. Consider gmtime_r or C++20 <chrono>."
              << std::endl;
#endif
    char datetime[32];
    std::strftime(datetime, sizeof(datetime), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);

    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <dc:creator>DuckX</dc:creator>
    <dcterms:created xsi:type="dcterms:W3CDTF">)" +
           std::string(datetime) + R"(</dcterms:created>
    <dcterms:modified xsi:type="dcterms:W3CDTF">)" +
           std::string(datetime) + R"(</dcterms:modified>
</cp:coreProperties>)";
}

std::string duckx::Document::get_document_rels_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
</Relationships>)";
}

std::string duckx::Document::get_empty_document_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
    <w:body>
    </w:body>
</w:document>)";
}

void duckx::Document::file(std::string directory) { this->directory = std::move(directory); }

void duckx::Document::open()
{
    void* buf = nullptr;
    size_t bufsize;

    // Open file and load "xml" content to the document variable
    zip_t* zip = zip_open(this->directory.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');

    if (!zip)
    {
        this->flag_is_open = false;
        return;
    }
    this->flag_is_open = true;

    zip_entry_open(zip, "word/document.xml");
    zip_entry_read(zip, &buf, &bufsize);

    zip_entry_close(zip);
    zip_close(zip);

    this->document.load_buffer(buf, bufsize);

    free(buf);

    this->paragraph.set_parent(document.child("w:document").child("w:body"));
}

void duckx::Document::save() const
{
    // minizip only supports appending or writing to new files
    // so we must
    // - make a new file
    // - write any new files
    // - copy the old files
    // - delete old docx
    // - rename new file to old file

    if (!this->is_open())
    {
        // if file is not existing, save() will make no sense
        return;
    }

    // Read document buffer
    xml_string_writer writer;
    this->document.print(writer);

    // Open file and replace "xml" content

    const std::string original_file = this->directory;
    const std::string temp_file = this->directory + ".tmp";

    // Create the new file
    zip_t* new_zip = zip_open(temp_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

    // Write out document.xml
    zip_entry_open(new_zip, "word/document.xml");

    const char* buf = writer.result.c_str();

    zip_entry_write(new_zip, buf, strlen(buf));
    zip_entry_close(new_zip);

    // Open the original zip and copy all files which are not replaced by duckX
    zip_t* orig_zip = zip_open(original_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');

    // Loop & copy each relevant entry in the original zip
    const int orig_zip_entry_ct = zip_total_entries(orig_zip);
    for (int i = 0; i < orig_zip_entry_ct; i++)
    {
        zip_entry_openbyindex(orig_zip, i);
        const char* name = zip_entry_name(orig_zip);

        // Skip copying the original file
        if (std::string(name) != std::string("word/document.xml"))
        {
            // Read the old content
            void* entry_buf;
            size_t entry_buf_size;
            zip_entry_read(orig_zip, &entry_buf, &entry_buf_size);

            // Write into new zip
            zip_entry_open(new_zip, name);
            zip_entry_write(new_zip, entry_buf, entry_buf_size);
            zip_entry_close(new_zip);

            free(entry_buf);
        }

        zip_entry_close(orig_zip);
    }

    // Close both zips
    zip_close(orig_zip);
    zip_close(new_zip);

    // Remove original zip, rename new to correct name
    remove(original_file.c_str());
    rename(temp_file.c_str(), original_file.c_str());
}

bool duckx::Document::is_open() const { return this->flag_is_open; }

duckx::Paragraph& duckx::Document::paragraphs()
{
    pugi::xml_node body = document.child("w:document").child("w:body");

    // Check if body has any paragraph nodes
    if (!body.child("w:p"))
    {
        // If no paragraphs exist, create an initial paragraph
        pugi::xml_node new_para = body.append_child("w:p");
        // Add an empty run to make it a valid paragraph
        pugi::xml_node new_run = new_para.append_child("w:r");
        const pugi::xml_node new_text = new_run.append_child("w:t");
        new_text.text().set(""); // Empty text
    }

    // Set parent and return paragraph
    this->paragraph.set_parent(body);
    return this->paragraph;
}

duckx::Table& duckx::Document::tables()
{
    this->table.set_parent(document.child("w:document").child("w:body"));
    return this->table;
}
