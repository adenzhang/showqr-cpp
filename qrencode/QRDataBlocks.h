#pragma once
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <QrCode.hpp>
#include <iostream>
#include <fstream>

struct utils
{
    static const std::vector<std::vector<unsigned>> &getQRCapacities()
    {
        static std::vector<std::vector<unsigned>> s_cap = {
                {17, 14, 11, 7},          {32, 26, 20, 14},         {53, 42, 32, 24},         {78, 62, 46, 34},         {106, 84, 60, 44},
                {134, 106, 74, 58},       {154, 122, 86, 64},       {192, 152, 108, 84},      {230, 180, 130, 98},      {271, 213, 151, 119},
                {321, 251, 177, 137},     {367, 287, 203, 155},     {425, 331, 241, 177},     {458, 362, 258, 194},     {520, 412, 292, 220},
                {586, 450, 322, 250},     {644, 504, 364, 280},     {718, 560, 394, 310},     {792, 624, 442, 338},     {858, 666, 482, 382},
                {929, 711, 509, 403},     {1003, 779, 565, 439},    {1091, 857, 611, 461},    {1171, 911, 661, 511},    {1273, 997, 715, 535},
                {1367, 1059, 751, 593},   {1465, 1125, 805, 625},   {1528, 1190, 868, 658},   {1628, 1264, 908, 698},   {1732, 1370, 982, 742},
                {1840, 1452, 1030, 790},  {1952, 1538, 1112, 842},  {2068, 1628, 1168, 898},  {2188, 1722, 1228, 958},  {2303, 1809, 1283, 983},
                {2431, 1911, 1351, 1051}, {2563, 1989, 1423, 1093}, {2699, 2099, 1499, 1139}, {2809, 2213, 1579, 1219}, {2953, 2331, 1663, 1273}};
        return s_cap;
    }
    static size_t getBase64Size( size_t n )
    {
        return ( n + 2 ) / 3 * 4;
    }

    // version: 1 - 40
    // correctionLevel: L:0, M: 1, Q: 2, H: 3
    static size_t get_capacity( unsigned version, int correctionLevel )
    {
        return getQRCapacities()[version - 1][correctionLevel];
    }
    static size_t count_digits( size_t x )
    {
        return std::to_string( x ).length();
    }
    static size_t header_size( size_t nblocks, size_t headersufix_size )
    {
        return count_digits( nblocks ) + headersufix_size;
    }
    static size_t ceilDiv( size_t x, size_t y )
    {
        return ( x + y - 1 ) / y;
    }
    static int calc_blocksize( bool isB64Mode, size_t nblocks, size_t blockcap, size_t datasize, size_t headersufix_size )
    {
        //            """ return blocksize or -1 """
        size_t blksize = ceilDiv( datasize, nblocks );
        if ( ( isB64Mode ? getBase64Size( blksize ) : blksize ) + header_size( nblocks + 1, headersufix_size ) <=
             blockcap ) // plus the first meta block
            return blksize;
        return -1;
    }
};

template<bool bWithPadding = true>
struct base64
{
    static constexpr const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                "abcdefghijklmnopqrstuvwxyz"
                                                "0123456789+/";


    static inline bool is_base64( unsigned char c )
    {
        return ( isalnum( c ) || ( c == '+' ) || ( c == '/' ) );
    }

    template<class CharOrByte>
    static std::string encodeToString( CharOrByte const *bytes_to_encode, unsigned int in_len )
    {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while ( in_len-- )
        {
            char_array_3[i++] = *( bytes_to_encode++ );
            if ( i == 3 )
            {
                char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
                char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
                char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
                char_array_4[3] = char_array_3[2] & 0x3f;

                for ( i = 0; ( i < 4 ); i++ )
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if ( i )
        {
            for ( j = i; j < 3; j++ )
                char_array_3[j] = '\0';

            char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
            char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
            char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );

            for ( j = 0; ( j < i + 1 ); j++ )
                ret += base64_chars[char_array_4[j]];

            while ( ( i++ < 3 ) )
                ret += '=';
        }

        return ret;
    }
};

template<class T, class Size = size_t>
struct Buffer
{
    T *data = nullptr;
    Size len = 0;

    Buffer( T *d, size_t n ) : data( d ), len( n )
    {
    }
    size_t size() const
    {
        return len;
    }
};
using CharBuffer = Buffer<char>;
using ConstCharBuffer = Buffer<const char>;
using ByteBuffer = Buffer<std::uint8_t>;

struct BlockDataProvider
{
    virtual void setBlockSize( unsigned blocksize ) = 0;
    virtual size_t getDataSize() = 0;
    virtual ConstCharBuffer getBlock( size_t iblock ) = 0; // byte[].length == blocksize except for the last block.
    virtual void release() = 0;

    virtual ~BlockDataProvider() = default;
};
using BlockDataProviderPtr = std::shared_ptr<BlockDataProvider>;

struct FileDataProvider : public BlockDataProvider
{
    std::ifstream mFile;
    size_t mBlockSize = 0, mBlockCount = 0, mSize = 0, mLastBlockSize = 0;
    std::vector<char> mBuf;

    FileDataProvider();

    FileDataProvider( const std::string &filename, unsigned blocksize = 1 )
    {
        init( filename, blocksize );
    }

    void init( const std::string &filename, unsigned blocksize = 1 )
    {
        release();
        mFile.open( filename );
        if ( !mFile.is_open() )
        {
            throw std::runtime_error( "Failed to open file " + filename );
        }
        mFile.seekg( 0, std::ios::end );
        mSize = mFile.tellg();
        setBlockSize( blocksize );
    }
    void setBlockSize( unsigned n ) override
    {
        mBlockSize = n;
        mBlockCount = utils::ceilDiv( mSize, mBlockSize );
        mBuf.resize( mBlockSize );
        if ( mBlockCount * mBlockSize == mBlockSize )
            mLastBlockSize = mBlockSize;
        else
            mLastBlockSize = ( mSize - ( mBlockCount - 1 ) * mBlockSize );
    }
    size_t getDataSize() override
    {
        return mSize;
    }
    ConstCharBuffer getBlock( size_t iblock ) override
    {
        mFile.seekg( iblock * mBlockSize );
        if ( iblock + 1 >= mBlockCount )
        {
            mFile.read( &mBuf[0], mLastBlockSize );
            return {&mBuf[0], mLastBlockSize};
        }
        else
        {
            mFile.read( &mBuf[0], mBlockSize );
            return {&mBuf[0], mBlockSize};
        }
    }
    void release() override
    {
        if ( mFile.is_open() )
            mFile.close();
    }
    ~FileDataProvider() override
    {
        release();
    }
};

struct IGraphic
{
    virtual size_t getWidth() const = 0;
    virtual size_t getHeight() const = 0;
    virtual void drawRect(
            size_t left, size_t top, size_t width, size_t height, unsigned char colorR, unsigned char colorG, unsigned char colorB ) = 0;
    virtual ~IGraphic() = default;
};

// called by UI framework.
struct IQrDataRender
{
    virtual bool prepareDataBlock( int ilbock ) = 0;
    // return pixel width per QR module. -1 for error
    virtual int renderCurrentBlock( IGraphic &g ) = 0;
    virtual ~IQrDataRender() = default;
};

template<class QREncoder = qrcodegen::QrCode, class Base64 = base64<>>
class QRDataBlocks : public IQrDataRender
{
public:
    std::string mFileName, mFileName_b64;
    BlockDataProviderPtr mBlocks;
    unsigned mQRVersion = 40;

    size_t mDataSize; // from DataBlocks.getDataSize()

    size_t mBlockCount; // inferred
    size_t mBlockSize; // inferred, per block data size
    size_t mHeaderSize; // "234|234FA|"

    std::string mStreamID;
    std::string mHeaderFmt;
    bool mIsBinaryMode =
            true; // binary mode or base64 mode. For binary mode, payload will be bytes, header is still text. first Meta frame is still text mode.

    std::vector<std::uint8_t> mDataBuf; // store the last buffer
    std::string mStrBuf; // store the last buffer

    QREncoder mQrCode;
    size_t quietZone = 1;

    QRDataBlocks()
    {
    }
    QRDataBlocks( const std::string &filename, BlockDataProviderPtr dataBlocks, unsigned qrversion )
    {
        init( filename, dataBlocks, qrversion );
    }

    void init( const std::string &filename, BlockDataProviderPtr dataBlocks, unsigned qrversion )
    {
        { // basename
            auto pos = filename.find_last_of( "\\/" );
            if ( std::string::npos != pos )
                mFileName = filename.substr( pos + 1 );
            else
                mFileName = filename;
        }
        mFileName_b64 = Base64::encodeToString( mFileName.c_str(), mFileName.size() );
        mBlocks = dataBlocks;

        mQRVersion = qrversion;

        mDataSize = dataBlocks->getDataSize();

        int blockcap = utils::get_capacity( mQRVersion, 0 );

        {
            std::stringstream ss;
            ss << std::hex << rand();
            mStreamID = ss.str();
        }

        calc_blocksize( mDataSize, blockcap, mStreamID.length() + 2 ); // format "|1234abc|"
        dataBlocks->setBlockSize( mBlockSize );

        mHeaderFmt = "%0" + std::to_string( mHeaderSize - mStreamID.length() - 2 ) + "d|%s|";
    }

    void updateVersion( int qrversion )
    {
        init( mFileName, mBlocks, qrversion );
    }

    std::string getHeader( int iblock )
    {
        char buf[32];
        sprintf( buf, mHeaderFmt.c_str(), iblock, mStreamID.c_str() );
        return buf;
    }

    // @return base64 encoded string
    std::string &getBlockString( int iblock )
    {
        std::stringstream ss;
        if ( iblock == 0 )
        {
            // meta block format: 'nb=nblocks|bs=blocksize|fs=filesize|fn=filename'
            ss << getHeader( iblock ) << "nb=" << mBlockCount << "|bs=" << mBlockSize << "|fs=" << mDataSize
               << "|mod=" << ( mIsBinaryMode ? "bin" : "b64" ) << "|fn=" << mFileName_b64;
        }
        else
        {
            auto buffer = mBlocks->getBlock( iblock - 1 );
            ss << getHeader( iblock ) << Base64::encodeToString( buffer.data, buffer.len );
        }
        return mStrBuf = ss.str();
    }
    // @return string header and row bytes block.
    std::vector<std::uint8_t> &getBlockBytes( int iblock )
    {
        mDataBuf.clear();
        if ( iblock > 0 )
        { // use getBlock when iblock == 0.
            auto header = getHeader( iblock );
            auto payload = mBlocks->getBlock( iblock - 1 );
            mDataBuf.reserve( header.size() + payload.size() );
            mDataBuf.insert( mDataBuf.end(), header.begin(), header.end() );
            mDataBuf.insert( mDataBuf.end(), payload.data, payload.data + payload.size() );
        }
        else
        {
            auto s = getBlockString( 0 );
            mDataBuf.reserve( s.size() );
            mDataBuf.insert( mDataBuf.end(), s.begin(), s.end() );
        }
        return mDataBuf;
    }

    bool prepareDataBlock( int iblock ) override
    {
        // encode data block so that it can be rendered.
        if ( mIsBinaryMode )
        {
            if ( iblock > 0 )
            {
                auto &b = getBlockBytes( iblock );
                mQrCode = QREncoder::encodeBinary( b, QREncoder::Ecc::LOW );
            }
            else
            {
                auto &b = getBlockString( 0 );
                mQrCode = QREncoder::encodeText( b.c_str(), QREncoder::Ecc::LOW );
            }
        }
        else
        {
            auto &s = getBlockString( iblock );
            mQrCode = QREncoder::encodeText( s.c_str(), QREncoder::Ecc::LOW );
        }
        return true;
    }

    // return pixel width per QR module. -1 for error
    int renderCurrentBlock( IGraphic &g ) override
    {
        auto W = std::min( g.getHeight(), g.getWidth() ); // picture size
        size_t N = mQrCode.getSize(); // QR size
        if ( quietZone * 2 + N * 2 > W )
        {
            std::cerr << "Too small image to paint qrcode: " << quietZone * 2 + N * 2 << " >= " << W;
            return -1;
        }
        size_t boxsize = ( W - 2 * quietZone ) / N;
        size_t cx = ( g.getWidth() - boxsize * N ) / 2, cy = ( g.getHeight() - boxsize * N ) / 2; // place image at center.
        g.drawRect( 0, 0, g.getWidth(), g.getHeight(), 255, 255, 255 ); // white
        for ( size_t r = 0; r < N; ++r )
        {
            for ( size_t c = 0; c < N; ++c )
            {
                if ( mQrCode.getModule( r, c ) )
                    g.drawRect( cx + c * boxsize, cy + r * boxsize, boxsize, boxsize, 0, 0, 0 );
            }
        }
        return boxsize;
    }
    int getQRModuleSize()
    {
        return mQrCode.getSize();
    }

    //    give raw data size, b64 encode it and split it into blocks. Adding header to each block, so that the block size <= blockcap.
    //    header format 'blockindex|crc32cfilehecksum_of_file|',
    //    where blockindex and checksum have fixed number of digits like '002|2A35AC23|'
    //    headersufix_size: size of '|2A35AC23|'
    //    first block is in format '0|crc32cfilehecksum_of_file|nb:nblocks,bs:blocksize,fs:filesize,b64:[ALL|BLOCK|NONE],fn:filename',
    //    where nblocks doesn't include the first block.
    //    b64: ALL, encode whole data; BLOCK: encode per block; NONE, no b64 encode, no implemented right now.
    //    Here b64=BLOCK is default.
    //    return (nblocks, headersize, blocksize)
    void calc_blocksize( size_t datasize, size_t blockcap, size_t headersufix_size )
    {
        size_t nblocks = utils::ceilDiv( datasize, blockcap );
        int blocksize = -1;
        while ( true )
        {
            blocksize = utils::calc_blocksize( !mIsBinaryMode, nblocks, blockcap, datasize, headersufix_size );
            if ( blocksize >= 0 )
                break;
            nblocks += 1;
        }
        mBlockCount = nblocks + 1;
        mHeaderSize = utils::header_size( nblocks + 1, headersufix_size );
        mBlockSize = blocksize;
    }
};

//////////////////////////////////////////////////////////////////////////

struct QRUIControl
{
    virtual void print( const char *fmt, ... ) = 0;
    virtual void enableQRTimer( double seconds ) = 0;
    virtual void playStatusChanged( bool ) = 0;

    virtual void redraw() = 0;
    virtual void close() = 0;
    virtual ~QRUIControl() = default;
};
struct QrRenderControl
{
    int mBlockIdx = -1;
    QRDataBlocks<> mQrData;
    bool mStopped = true;
    double mPlayrateFPS = 2;

    int mUserKeyNumber = -1; //

    IGraphic *mGraph = nullptr;
    QRUIControl *mUIC = nullptr;

    void init( const char *filename )
    {
        mBlockIdx = -1;
        BlockDataProviderPtr pData = BlockDataProviderPtr( new FileDataProvider( filename ) );
        mQrData.init( filename, pData, mQrData.mQRVersion );
        mBlockIdx = 0;
    }
    bool valid() const
    {
        return mBlockIdx >= 0 && mGraph && mUIC;
    }

    bool flipActive()
    {
        if ( !valid() )
            return false;
        if ( mStopped )
        {
            if ( mBlockIdx < 0 )
            {
                mUIC->print( "Please select a file to encode!" );
                return !mStopped;
            }
            else
            {
                mUIC->enableQRTimer( 1.0 / mPlayrateFPS );
                mUIC->playStatusChanged( true );
            }
        }
        else
        {
            mUIC->playStatusChanged( false );
        }
        mStopped = !mStopped;
        return !mStopped;
    }
    bool prepareBlock()
    {
        if ( !valid() )
            return false;
        return mQrData.prepareDataBlock( mBlockIdx );
    }
    bool renderCurrentBlock()
    {
        if ( !valid() )
            return false;
        int pixelsPerModule = mQrData.renderCurrentBlock( *mGraph );
        // - show msg
        if ( mBlockIdx == 0 ) // header
        {
            size_t n = std::min( size_t( 50 ), mQrData.mStrBuf.size() );
            std::string s( mQrData.mStrBuf.begin(), std::next( mQrData.mStrBuf.begin(), n ) );
            mUIC->print( "%s, file:%s, rateFPS:%.1f, modulePixels:%d, modules:%d, version:%d",
                         s.c_str(),
                         mQrData.mFileName.c_str(),
                         mPlayrateFPS,
                         pixelsPerModule,
                         mQrData.getQRModuleSize(),
                         mQrData.mQrCode.getVersion() );
        }
        else if ( mQrData.mIsBinaryMode && mBlockIdx > 0 )
        {
            std::string s( mQrData.mDataBuf.begin(), std::next( mQrData.mDataBuf.begin(), mQrData.mHeaderSize ) );
            mUIC->print( "%s, rateFPS:%.1f, modulePixels:%d, modules:%d, version:%d, datasize:%d",
                         s.c_str(),
                         mPlayrateFPS,
                         pixelsPerModule,
                         mQrData.getQRModuleSize(),
                         mQrData.mQrCode.getVersion(),
                         mQrData.mDataBuf.size() );
        }
        else
        {
            size_t n = std::min( size_t( 50 ), mQrData.mStrBuf.size() );
            std::string s( mQrData.mStrBuf.begin(), std::next( mQrData.mStrBuf.begin(), n ) );
            mUIC->print( "%s, rateFPS:%.1f, modulePixels:%d, modules:%d, version:%d, datasize:%d",
                         s.c_str(),
                         mPlayrateFPS,
                         pixelsPerModule,
                         mQrData.getQRModuleSize(),
                         mQrData.mQrCode.getVersion(),
                         mQrData.mStrBuf.size() );
        }
        return true;
    }
    bool incBlock( bool bEnableTimer )
    {
        if ( mStopped )
            return false;
        ++mBlockIdx;
        if ( mBlockIdx >= mQrData.mBlockCount ) // completed
        {
            mBlockIdx = mQrData.mBlockCount;
            mStopped = true;
            mUIC->playStatusChanged( false );
        }
        else if ( bEnableTimer )
        {
            mUIC->enableQRTimer( 1.0 / mPlayrateFPS );
        }

        return true;
    }

    // user may need to convert to ascii code
    void onKeyEvent( unsigned char keyCodeASCII )
    {
        if ( !valid() )
            return;
        char ch = keyCodeASCII;
        if ( ch >= '0' && ch <= '9' )
        { // number
            if ( mUserKeyNumber != -1 )
                mUserKeyNumber = mUserKeyNumber * 10 + ( (int)ch ) - (int)'0';
            else
                mUserKeyNumber = ( (int)ch ) - (int)'0';
            return;
        }
        else if ( ch == '\n' )
        { // enter
            if ( mUserKeyNumber != -1 )
            {
                mBlockIdx = mUserKeyNumber;
                if ( mBlockIdx >= mQrData.mBlockCount )
                    mBlockIdx = mQrData.mBlockCount - 1;
                mUserKeyNumber = -1;
                prepareBlock();
                mUIC->redraw();
            }
            return;
        }
        else
        {
            mUserKeyNumber = -1;
        }
        switch ( ch )
        {
        case 'q': // exit
            mUIC->close();
            return;
        case ' ': // stop
            flipActive();
            return;
        case 'h': // home
            mBlockIdx = 0;
            break;
        case 'e': // home
            mBlockIdx = mQrData.mBlockCount - 1;
            break;
        case 'l': // next frame/block
            ++mBlockIdx;
            if ( mBlockIdx >= mQrData.mBlockCount )
                mBlockIdx = mQrData.mBlockCount - 1;
            break;
        case 'j': // previous fram/block
            --mBlockIdx;
            if ( mBlockIdx < 0 )
                mBlockIdx = 0;
            break;
        case 'i': // speed up
            mPlayrateFPS = mPlayrateFPS + ( mPlayrateFPS > 1 ? 0.5 : 0.1 );
            break;
        case 'k': // slow down
            mPlayrateFPS = mPlayrateFPS - ( mPlayrateFPS > 1 ? 0.5 : 0.1 );
            if ( mPlayrateFPS < 0.1 )
                mPlayrateFPS = 0.1;
            break;
        case 'w': // inc version
            if ( mQrData.mQRVersion + 1 <= 40 )
            {
                mQrData.updateVersion( mQrData.mQRVersion + 1 );
                //                            mBlockIdx = 0;
            }
            break;
        case 's': // dec version
            if ( mQrData.mQRVersion - 1 >= 4 )
            {
                mQrData.updateVersion( mQrData.mQRVersion - 1 );
                //                            mBlockIdx = 0;
            }
            break;
            //        case 'd': // inc imgsize
            //            mImageSize += 50;
            //            mQrData.setImageSize( mImageSize );
            //            break;
            //        case 'a': // dec imgsize
            //            if ( mImageSize - 50 > 200 )
            //            {
            //                mImageSize -= 50;
            //                mQrData.setImageSize( mImageSize );
            //            }
            //            break;
        }
        prepareBlock();
        mUIC->redraw();
    }
};
