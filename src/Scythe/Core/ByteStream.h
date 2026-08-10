#pragma once
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Scythe
{
    enum class StreamMode { Saving, Loading };
    
    class ByteStream
    {
    public:
        virtual ~ByteStream() = default;
        
        bool IsSaving()  const { return m_Mode == StreamMode::Saving; }
        bool IsLoading() const { return m_Mode == StreamMode::Loading; }
        
        uint32_t GetVersion() const { return m_Version; }
        void SetVersion(uint32_t v) { m_Version = v; }
        
        virtual void TransferBytes(void* data, size_t size) = 0;
        
        template <typename T>
            requires std::is_trivially_copyable_v<T>
        ByteStream& operator<<(T& value)
        {
            TransferBytes(&value, sizeof(T));
            return *this;
        }
        
        ByteStream& operator<<(std::string& str)
        {
            uint32_t length = static_cast<uint32_t>(str.size());
            *this << length;                      // writes OR reads the length
            if (IsLoading())
                str.resize(length);
            TransferBytes(str.data(), length);    // then the chars
            return *this;
        }
    protected:
        ByteStream(StreamMode mode, uint32_t version)
            : m_Mode(mode), m_Version(version) {}
        StreamMode m_Mode;
        uint32_t m_Version;
    };
    
    class MemoryWriter : public ByteStream
    {
    public:
        MemoryWriter() : ByteStream(StreamMode::Saving, 1) {}
        
        void TransferBytes(void* data, size_t size) override
        {
            const uint8_t* p = static_cast<const uint8_t*>(data);
            m_Buffer.insert(m_Buffer.end(), p, p + size);
        }
        
        const std::vector<uint8_t>& GetBuffer() const { return m_Buffer; }
        
        void WriteToFile(const std::string& path) const
        {
            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            if (!file) throw std::runtime_error("MemoryWriter: cannot open: " + path);
            file.write(reinterpret_cast<const char*>(m_Buffer.data()),
                       static_cast<std::streamsize>(m_Buffer.size()));
        }
    private:
        std::vector<uint8_t> m_Buffer;
    };
    
    class MemoryReader : public ByteStream
    {
    public:
        explicit MemoryReader(std::vector<uint8_t> data)
            : ByteStream(StreamMode::Loading, 1), m_Buffer(std::move(data)) {}

        static MemoryReader FromFile(const std::string& path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) throw std::runtime_error("MemoryReader: cannot open: " + path);
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(static_cast<size_t>(size));
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            return MemoryReader(std::move(buffer));
        }

        void TransferBytes(void* data, size_t size) override
        {
            if (m_Offset + size > m_Buffer.size())
                throw std::runtime_error("MemoryReader: read past end of buffer");
            std::memcpy(data, m_Buffer.data() + m_Offset, size); // no alignment UB
            m_Offset += size;
        }

    private:
        std::vector<uint8_t> m_Buffer;
        size_t m_Offset = 0;
    };
}
