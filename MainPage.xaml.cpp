//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <Windows.h>
#include <Mfidl.h>

using namespace MediaStreamSource;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Playback;
using namespace Windows::Media::Protection;
using namespace Windows::Media::Protection::PlayReady;
using namespace Windows::Security::Cryptography;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Raw samples converted into static byte arrays (xxd -i)
#include "sintel_audio_clear_bin.h"
#include "sintel_audio_encrypted_bin.h"

// manifest urls for both assets
// http://playready.directtaps.net/media/hls/sintelh264aac/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)
// http://playready.directtaps.net/media/hls/sintelh264aacenc/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)

// this is the <WRMHEADER> extracted from the URL above
unsigned char playready_header[] = {
	0x3e, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x34, 0x03, 0x3c, 0x00,
	0x57, 0x00, 0x52, 0x00, 0x4d, 0x00, 0x48, 0x00, 0x45, 0x00, 0x41, 0x00,
	0x44, 0x00, 0x45, 0x00, 0x52, 0x00, 0x20, 0x00, 0x78, 0x00, 0x6d, 0x00,
	0x6c, 0x00, 0x6e, 0x00, 0x73, 0x00, 0x3d, 0x00, 0x22, 0x00, 0x68, 0x00,
	0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x2f, 0x00,
	0x73, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x61, 0x00,
	0x73, 0x00, 0x2e, 0x00, 0x6d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,
	0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x2e, 0x00,
	0x63, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x2f, 0x00, 0x44, 0x00, 0x52, 0x00,
	0x4d, 0x00, 0x2f, 0x00, 0x32, 0x00, 0x30, 0x00, 0x30, 0x00, 0x37, 0x00,
	0x2f, 0x00, 0x30, 0x00, 0x33, 0x00, 0x2f, 0x00, 0x50, 0x00, 0x6c, 0x00,
	0x61, 0x00, 0x79, 0x00, 0x52, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00,
	0x79, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00,
	0x72, 0x00, 0x22, 0x00, 0x20, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00,
	0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x3d, 0x00, 0x22, 0x00,
	0x34, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00,
	0x30, 0x00, 0x22, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x44, 0x00, 0x41, 0x00,
	0x54, 0x00, 0x41, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x50, 0x00, 0x52, 0x00,
	0x4f, 0x00, 0x54, 0x00, 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00,
	0x4e, 0x00, 0x46, 0x00, 0x4f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x4b, 0x00,
	0x45, 0x00, 0x59, 0x00, 0x4c, 0x00, 0x45, 0x00, 0x4e, 0x00, 0x3e, 0x00,
	0x31, 0x00, 0x36, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x4b, 0x00, 0x45, 0x00,
	0x59, 0x00, 0x4c, 0x00, 0x45, 0x00, 0x4e, 0x00, 0x3e, 0x00, 0x3c, 0x00,
	0x41, 0x00, 0x4c, 0x00, 0x47, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3e, 0x00,
	0x41, 0x00, 0x45, 0x00, 0x53, 0x00, 0x43, 0x00, 0x54, 0x00, 0x52, 0x00,
	0x3c, 0x00, 0x2f, 0x00, 0x41, 0x00, 0x4c, 0x00, 0x47, 0x00, 0x49, 0x00,
	0x44, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x50, 0x00, 0x52, 0x00,
	0x4f, 0x00, 0x54, 0x00, 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00,
	0x4e, 0x00, 0x46, 0x00, 0x4f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x4b, 0x00,
	0x49, 0x00, 0x44, 0x00, 0x3e, 0x00, 0x71, 0x00, 0x68, 0x00, 0x6d, 0x00,
	0x63, 0x00, 0x52, 0x00, 0x2f, 0x00, 0x75, 0x00, 0x75, 0x00, 0x4c, 0x00,
	0x6b, 0x00, 0x69, 0x00, 0x44, 0x00, 0x4e, 0x00, 0x77, 0x00, 0x2f, 0x00,
	0x31, 0x00, 0x75, 0x00, 0x6a, 0x00, 0x71, 0x00, 0x45, 0x00, 0x6f, 0x00,
	0x77, 0x00, 0x3d, 0x00, 0x3d, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x4b, 0x00,
	0x49, 0x00, 0x44, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x43, 0x00, 0x48, 0x00,
	0x45, 0x00, 0x43, 0x00, 0x4b, 0x00, 0x53, 0x00, 0x55, 0x00, 0x4d, 0x00,
	0x3e, 0x00, 0x58, 0x00, 0x66, 0x00, 0x4b, 0x00, 0x36, 0x00, 0x41, 0x00,
	0x54, 0x00, 0x75, 0x00, 0x6b, 0x00, 0x54, 0x00, 0x7a, 0x00, 0x41, 0x00,
	0x3d, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x43, 0x00, 0x48, 0x00, 0x45, 0x00,
	0x43, 0x00, 0x4b, 0x00, 0x53, 0x00, 0x55, 0x00, 0x4d, 0x00, 0x3e, 0x00,
	0x3c, 0x00, 0x4c, 0x00, 0x41, 0x00, 0x5f, 0x00, 0x55, 0x00, 0x52, 0x00,
	0x4c, 0x00, 0x3e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00,
	0x3a, 0x00, 0x2f, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x6c, 0x00, 0x61, 0x00,
	0x79, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x79, 0x00,
	0x2e, 0x00, 0x64, 0x00, 0x69, 0x00, 0x72, 0x00, 0x65, 0x00, 0x63, 0x00,
	0x74, 0x00, 0x74, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x2e, 0x00,
	0x6e, 0x00, 0x65, 0x00, 0x74, 0x00, 0x2f, 0x00, 0x70, 0x00, 0x72, 0x00,
	0x2f, 0x00, 0x73, 0x00, 0x76, 0x00, 0x63, 0x00, 0x2f, 0x00, 0x72, 0x00,
	0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x73, 0x00, 0x6d, 0x00,
	0x61, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 0x00,
	0x2e, 0x00, 0x61, 0x00, 0x73, 0x00, 0x6d, 0x00, 0x78, 0x00, 0x3c, 0x00,
	0x2f, 0x00, 0x4c, 0x00, 0x41, 0x00, 0x5f, 0x00, 0x55, 0x00, 0x52, 0x00,
	0x4c, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x43, 0x00, 0x55, 0x00, 0x53, 0x00,
	0x54, 0x00, 0x4f, 0x00, 0x4d, 0x00, 0x41, 0x00, 0x54, 0x00, 0x54, 0x00,
	0x52, 0x00, 0x49, 0x00, 0x42, 0x00, 0x55, 0x00, 0x54, 0x00, 0x45, 0x00,
	0x53, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x49, 0x00, 0x49, 0x00, 0x53, 0x00,
	0x5f, 0x00, 0x44, 0x00, 0x52, 0x00, 0x4d, 0x00, 0x5f, 0x00, 0x56, 0x00,
	0x45, 0x00, 0x52, 0x00, 0x53, 0x00, 0x49, 0x00, 0x4f, 0x00, 0x4e, 0x00,
	0x3e, 0x00, 0x37, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x35, 0x00, 0x36, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x34, 0x00, 0x3c, 0x00,
	0x2f, 0x00, 0x49, 0x00, 0x49, 0x00, 0x53, 0x00, 0x5f, 0x00, 0x44, 0x00,
	0x52, 0x00, 0x4d, 0x00, 0x5f, 0x00, 0x56, 0x00, 0x45, 0x00, 0x52, 0x00,
	0x53, 0x00, 0x49, 0x00, 0x4f, 0x00, 0x4e, 0x00, 0x3e, 0x00, 0x3c, 0x00,
	0x2f, 0x00, 0x43, 0x00, 0x55, 0x00, 0x53, 0x00, 0x54, 0x00, 0x4f, 0x00,
	0x4d, 0x00, 0x41, 0x00, 0x54, 0x00, 0x54, 0x00, 0x52, 0x00, 0x49, 0x00,
	0x42, 0x00, 0x55, 0x00, 0x54, 0x00, 0x45, 0x00, 0x53, 0x00, 0x3e, 0x00,
	0x3c, 0x00, 0x2f, 0x00, 0x44, 0x00, 0x41, 0x00, 0x54, 0x00, 0x41, 0x00,
	0x3e, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x57, 0x00, 0x52, 0x00, 0x4d, 0x00,
	0x48, 0x00, 0x45, 0x00, 0x41, 0x00, 0x44, 0x00, 0x45, 0x00, 0x52, 0x00,
	0x3e, 0x00
};

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

/*
 * This is an internal struct taken from our depacketizer that contains
 * various info about the sample: CTS, IV, subsamples...
 * Both arrays 'sintel_audio_clear_bin' and 'sintel_audio_encrypted_bin' are 
 * filled with those (AU, data, AU, data, AU, data, ...).
 */
struct AU
{
	unsigned int       m_streamID;             /**< Id of the stream to get an AU from */
	unsigned char     *m_dataAddress;          /**< Pointer to a memory area with the encoded data */
	unsigned int       m_size;                 /**< Size of the dataAdress area */
	double             m_CTS;                  /**< Composition Time Stamp for the Access Unit */
	double             m_DTS;                  /**< Decoded Time Stamp for the Access Unit */
	unsigned char      m_attribute;            /**< RAP information & AU corrupted */
	unsigned int       m_maxsize;              /**< Maximum size of the AU */
	unsigned int       m_structSize;           /**< Structure size */
	unsigned int       m_sizeStartCode;
	unsigned long long m_IV;
	unsigned long long m_IVBlockOffset;
	unsigned short     m_uiSubSample_count;
	unsigned short     m_puiBytesOfClearData[100];
	unsigned int       m_puiBytesOfEncryptedData[100];
	int                m_i32UtcTimeSeconds;    /**< UTC time depacked from stream if available */
	unsigned char      m_pucKID[16];           /**< KID for cenc streams */
};


MainPage::MainPage() :
	m_mediaPlayer(nullptr),
	m_buffer(nullptr), 
	m_bufferSize(0), 
	m_currentAU(nullptr)
{
	InitializeComponent();

	// data pointers are wrong in those arrays
	FixAddresses(sintel_audio_clear_bin, sintel_audio_clear_bin_len);
	FixAddresses(sintel_audio_encrypted_bin, sintel_audio_encrypted_bin_len);

	// media protection manager (we'll use the same for everybody - is that wrong ?)
	m_mediaProtectionManager = ref new MediaProtectionManager();
	PropertySet ^props = ref new PropertySet();
	props->Insert("{F4637010-03C3-42CD-B932-B48ADF3A6A54}", "Windows.Media.Protection.PlayReady.PlayReadyWinRTTrustedInput");
	m_mediaProtectionManager->Properties->Insert("Windows.Media.Protection.MediaProtectionSystemIdMapping", props);
	m_mediaProtectionManager->Properties->Insert("Windows.Media.Protection.MediaProtectionSystemId", "{F4637010-03C3-42CD-B932-B48ADF3A6A54}");
	m_mediaProtectionManager->Properties->Insert("Windows.Media.Protection.MediaProtectionContainerGuid", "{9A04F079-9840-4286-AB92-E65BE0885F95}");
	m_mediaProtectionManager->ServiceRequested += ref new ServiceRequestedEventHandler(this, &MainPage::ServiceRequested);
	m_mediaProtectionManager->ComponentLoadFailed += ref new ComponentLoadFailedEventHandler(this, &MainPage::ComponentLoadFailed);
	m_mediaProtectionManager->RebootNeeded += ref new RebootNeededEventHandler(this, &MainPage::RebootNeeded);
}

/*
 * This one works flawlessly.
 */
void MainPage::PlayClearSamples(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_buffer = sintel_audio_clear_bin;
	m_bufferSize = sintel_audio_clear_bin_len;
	m_currentAU = (AU *) m_buffer;

	// audio properties that "just work" with those samples
	AudioEncodingProperties ^audioProps = AudioEncodingProperties::CreateAacAdts(44100, 1, 72000);
	AudioStreamDescriptor ^audioStream = ref new AudioStreamDescriptor(audioProps);
	DumpProperties(audioStream->EncodingProperties->Properties);

	Windows::Media::Core::MediaStreamSource ^streamSource = ref new Windows::Media::Core::MediaStreamSource(audioStream);
	streamSource->SampleRequested += ref new TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, MediaStreamSourceSampleRequestedEventArgs ^>(this, &MainPage::SampleRequested);
	streamSource->Starting += ref new TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, MediaStreamSourceStartingEventArgs ^>(this, &MainPage::Starting);

	// and play
	CreateMediaPlayer();
	m_mediaPlayer->Source = MediaSource::CreateFromMediaStreamSource(streamSource);
	m_mediaPlayer->Play();
}

/*
 * This is what we're tring to do, without success.
 * These samples have been taken from a PR-encrypted version of the same
 * stream (both URLs on top of this file).
 * We are going to assume it's the same data, but encrypted. Not really 
 * important since we're not even going to SampleRequested (nor Starting, for
 * that matter).
 */
void MainPage::PlayEncryptedSamples(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_buffer = sintel_audio_encrypted_bin;
	m_bufferSize = sintel_audio_encrypted_bin_len;
	m_currentAU = (AU *)m_buffer;

	// audio properties that "just work" with those samples
	AudioEncodingProperties ^audioProps = AudioEncodingProperties::CreateAacAdts(44100, 1, 72000);

	// adding the MF_SD_PROTECTED property as advised in the mail thread
	audioProps->Properties->Insert(MF_SD_PROTECTED, PropertyValue::CreateUInt32(1));

	// stream descriptor
	AudioStreamDescriptor ^audioStream = ref new AudioStreamDescriptor(audioProps);
	DumpProperties(audioStream->EncodingProperties->Properties);

	Windows::Media::Core::MediaStreamSource ^streamSource = ref new Windows::Media::Core::MediaStreamSource(audioStream);
	streamSource->SampleRequested += ref new TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, MediaStreamSourceSampleRequestedEventArgs ^>(this, &MainPage::SampleRequested);
	streamSource->Starting += ref new TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, MediaStreamSourceStartingEventArgs ^>(this, &MainPage::Starting);

	
	/*
	 * Here, we are trying to build the binary blob discussed in the mail thread
	 */

	// Guids I tried:
	//Platform::Guid guidCPSystemId(0xF4637010, 0x03C3, 0x42CD, 0xB9, 0x32, 0xB4, 0x8A, 0xDF, 0x3A, 0x6A, 0x54); // works. same as PlayReadyStatics::MediaProtectionSystemId used below
	//Platform::Guid guidCPSystemId(0x9A04F079, 0x9840, 0x4286, 0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95); // not this one -> 80070057

	// this is a handy class that seem to the trick
	PlayReadyITADataGenerator ^generator = ref new PlayReadyITADataGenerator();

	// in addition to the guid, we need a PropertySet.
	// from the doc:
	//	 IPropertySet may contain any of the following values but must contain at least one of them.
	//
	//	 The property N, where N is replaced by the base-10 stream number being decrypted, set to the PlayReady Object corresponding to that stream.
	//	 The property set to a PlayReady Object that will be used for any stream number that was not set using N as described above.
	// Mkay...

	PropertySet ^properties = ref new PropertySet();

	// to fill the PropertySet, I tried several things: 
	// PlayReadyContentHeader object: nope. it doesn't even seem to be serialized to the blob.
	// WRMHEADER as a string: seems to land in the blob

	//PlayReadyContentHeader ^header = ref new PlayReadyContentHeader(ref new Array<unsigned char>(playready_header, sizeof(playready_header)));
	//String ^str = L"<WRMHEADER xmlns=\"http://schemasmicrosoftcom/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>qhmcR/uuLkiDNw/1ujqEow==</KID><CHECKSUM>XfK6ATukTzA=</CHECKSUM><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1565.4</IIS_DRM_VERSION></CUSTOMATTRIBUTES></DATA></WRMHEADER>";
	
	// what is the key, anyway ? I don't have such thing as a stream number.
	//properties->Insert("0", str);

	// at least with this, we go to SampleRequested
	auto blob = generator->GenerateData(PlayReadyStatics::MediaProtectionSystemId, 1, properties, PlayReadyITADataFormat::SerializedProperties_WithContentProtectionWrapper);
	m_mediaProtectionManager->Properties->Insert("Windows.Media.Protection.MediaProtectionSystemContext", blob);

	// as stated in the documentation, for a MediaStreamSource, the MediaProtectionManager must be set on the source
	streamSource->MediaProtectionManager = m_mediaProtectionManager;

	// and play
	CreateMediaPlayer();
	m_mediaPlayer->Source = MediaSource::CreateFromMediaStreamSource(streamSource);
	m_mediaPlayer->Play();
}

/*
 * Control method: the clear URL plays
 */
void MainPage::PlayClearURL(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	CreateMediaPlayer();
	m_mediaPlayer->Source = MediaSource::CreateFromUri(ref new Uri(L"http://playready.directtaps.net/media/hls/sintelh264aac/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)"));
	m_mediaPlayer->Play();
}

/*
 * Control method: The media protection manager works.
 */
void MainPage::PlayEncryptedURL(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	CreateMediaPlayer();
	m_mediaPlayer->ProtectionManager = m_mediaProtectionManager;

	// for some reason, the original URL does not play, let's use another one that works.
	// The point here is to prove the MPM is configured correctly.
	//m_mediaPlayer->Source = MediaSource::CreateFromUri(ref new Uri(L"http://playready.directtaps.net/media/hls/sintelh264aacenc/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)"));
	m_mediaPlayer->Source = MediaSource::CreateFromUri(ref new Uri(L"http://profficialsite.origin.mediaservices.windows.net/c51358ea-9a5e-4322-8951-897d640fdfd7/tearsofsteel_4k.ism/manifest(format=mpd-time-csf)"));
	m_mediaPlayer->Play();
}

void MainPage::Stop(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	DestroyMediaPlayer();
}

void MainPage::MediaOpened(Windows::Media::Playback::MediaPlayer^ mediaPlayer, Platform::Object^ args)
{
	Log("MediaOpened");
	for each (auto prop in m_mediaProtectionManager->Properties)
	{
		Log(prop->Key + ": " + prop->Value->ToString());
	}
}

void MainPage::MediaFailed(Windows::Media::Playback::MediaPlayer^ mediaPlayer, Windows::Media::Playback::MediaPlayerFailedEventArgs^ args)
{
	String ^error = "(unknown)";
	switch (args->Error) {
	case MediaPlayerError::Aborted:
		error = "Aborted";
		break;
	case MediaPlayerError::DecodingError:
		error = "DecodingError ";
		break;
	case MediaPlayerError::NetworkError:
		error = "NetworkError ";
		break;
	case MediaPlayerError::SourceNotSupported:
		error = "SourceNotSupported ";
		break;
	case MediaPlayerError::Unknown:
		error = "Unknown ";
		break;
	}
	wchar_t buffer[32]; // be large
	swprintf(buffer, sizeof(buffer), L"%x", args->ExtendedErrorCode.Value);
	Log("MediaFailed: Error = " + error + ", ErrorMessage = \"" + args->ErrorMessage + "\", ExtendedErrorCode = " + ref new String(buffer));
}

void MainPage::Starting(Windows::Media::Core::MediaStreamSource ^source, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args)
{
	auto request = args->Request;
	TimeSpan timeSpan;
	timeSpan.Duration = m_currentAU ? m_currentAU->m_CTS * 10000 : 0;
	request->SetActualStartPosition(timeSpan);

	Log("Starting: position = " + m_currentAU->m_CTS);
}

void MainPage::ServiceRequested(Windows::Media::Protection::MediaProtectionManager ^manager, Windows::Media::Protection::ServiceRequestedEventArgs ^args)
{
	Log("ServiceRequested");

	MediaProtectionServiceCompletion ^completion = args->Completion;
	PlayReadyIndividualizationServiceRequest ^individualizationRequest = nullptr;
	PlayReadyLicenseAcquisitionServiceRequest ^licenseAcquisitionRequest = nullptr;
	if ((individualizationRequest = dynamic_cast<PlayReadyIndividualizationServiceRequest ^>(args->Request)) != nullptr)
	{
		Log("PlayReadyIndividualizationServiceRequest start");
		create_task(individualizationRequest->BeginServiceRequest()).then([=]() {
			Log("PlayReadyIndividualizationServiceRequest::BeginServiceRequest done");
			completion->Complete(true);
		});
	}
	else if ((licenseAcquisitionRequest = dynamic_cast<PlayReadyLicenseAcquisitionServiceRequest ^>(args->Request)) != nullptr)
	{
		create_task(licenseAcquisitionRequest->BeginServiceRequest()).then([=]() {
			Log("PlayReadyLicenseAcquisitionServiceRequest done");
			Log("PlayReady HW supported: " + PlayReadyStatics::CheckSupportedHardware(PlayReadyHardwareDRMFeatures::HardwareDRM));
			Log("PlayReady security level: " + PlayReadyStatics::PlayReadyCertificateSecurityLevel);
			wchar_t buffer[16];
			swprintf(buffer, sizeof(buffer), L"%x", PlayReadyStatics::PlayReadySecurityVersion);
			Log("PlayReady version: " + ref new String(buffer));
			completion->Complete(true);
		});
	}
	else
	{
		// too bad... :(
		Log("ServiceRequested: unknown request " + args->Request);
	}
}

void MainPage::RebootNeeded(Windows::Media::Protection::MediaProtectionManager ^manager)
{
	Log("RebootNeeded");
}

void MainPage::ComponentLoadFailed(Windows::Media::Protection::MediaProtectionManager ^manager, Windows::Media::Protection::ComponentLoadFailedEventArgs ^args)
{
	Log("ComponentLoadFailed");
}

void MainPage::SampleRequested(Windows::Media::Core::MediaStreamSource ^source, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args)
{
	if (m_currentAU)
	{
		AU *au = m_currentAU;

		TimeSpan timeSpan;
		timeSpan.Duration = au->m_CTS * 10000;
		Array<byte> ^byteArray = ref new Array<byte>(au->m_dataAddress, au->m_size);
		IBuffer ^buffer = CryptographicBuffer::CreateFromByteArray(byteArray);
		MediaStreamSample ^sample = MediaStreamSample::CreateFromBuffer(buffer, timeSpan);
		if (au->m_uiSubSample_count)
		{
			// trying to set the key... (taken from the WRMHEADER on the top this file)
			// - as a byte array: seems to do nothing
			unsigned char keyId[] = { 0xaa, 0x19, 0x9c, 0x47, 0xfb, 0xae, 0x2e, 0x48, 0x83, 0x37, 0x0f, 0xf5, 0xba, 0x3a, 0x84, 0xa3 };

			// - base64-encoded: crashed the app once, needed to reboot to make it work again
			//unsigned char keyId[] = "qhmcR/uuLkiDNw/1ujqEow==";

			sample->Protection->SetKeyIdentifier(ref new Array<unsigned char>(keyId, sizeof(keyId)));

			// subsample mapping
			// tried as an array of integers, clear then encrypted then clear then encrypted, etc
			// the method takes a byte array, though...
			unsigned int *subSampleMapping = new unsigned int[au->m_uiSubSample_count * 2];
			for (int i = 0; i < au->m_uiSubSample_count; i++)
			{
				subSampleMapping[i * 2]     = au->m_puiBytesOfClearData[i];
				subSampleMapping[i * 2 + 1] = au->m_puiBytesOfEncryptedData[i];
			}
			sample->Protection->SetSubSampleMapping(ref new Array<unsigned char>((unsigned char *) subSampleMapping, au->m_uiSubSample_count * 2 * sizeof(unsigned int)));

			// IV: I tried big endian as I saw somewhere else
			// not sure about that.
			unsigned char iv[8];
			for (int i = 0; i < sizeof(iv) / sizeof(iv[0]); i++)
			{
				iv[i] = (au->m_IV >> ((7 - i) * 8)) & 0xff;
			}
			sample->Protection->SetInitializationVector(ref new Array<unsigned char>(iv, sizeof(iv)));
		}
		args->Request->Sample = sample;

		unsigned char *p = (unsigned char *) au;
		p += sizeof(AU);
		p += au->m_size;
		m_currentAU = (p - m_buffer) < m_bufferSize ? (AU *)p : nullptr;
	}
}

void MainPage::CreateMediaPlayer()
{
	if (m_mediaPlayer != nullptr)
	{
		DestroyMediaPlayer();
	}

	m_mediaPlayer = ref new MediaPlayer();
	m_mediaOpenedToken = m_mediaPlayer->MediaOpened += ref new TypedEventHandler<MediaPlayer ^, Object ^>(this, &MainPage::MediaOpened);
	m_mediaFailedToken = m_mediaPlayer->MediaFailed += ref new TypedEventHandler<MediaPlayer ^, MediaPlayerFailedEventArgs ^>(this, &MainPage::MediaFailed);
}

void MainPage::DestroyMediaPlayer()
{
	if (m_mediaPlayer != nullptr)
	{
		m_mediaPlayer->MediaOpened -= m_mediaOpenedToken;
		m_mediaPlayer->MediaFailed -= m_mediaFailedToken;
		m_mediaPlayer->Source = nullptr;
		m_mediaPlayer = nullptr;
	}
}

void MainPage::FixAddresses(unsigned char *buffer, unsigned int length)
{
	// m_dataAddress has to be recalculated because it has been 
	// read as is from the file and is completely irrelevant.
	// actual data is located immediately after the structure.
	unsigned char *p = buffer;
	unsigned char *end = buffer + length;
	while (p < end)
	{
		AU *au = (AU *)p;
		au->m_dataAddress = p + sizeof(AU);
		p = au->m_dataAddress + au->m_size;
	}
}

void MainPage::Log(String ^str)
{
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, str]() {
		String ^text = m_logBox->Text;
		text = str + "\n" + text;
		m_logBox->Text = text;
	}));
}

void MainPage::DumpProperties(Windows::Media::MediaProperties::MediaPropertySet ^properties)
{
	for each (auto prop in properties)
	{
		Guid guid = prop->Key;
		Object ^value = prop->Value;
		Log(guid.ToString() + " => " + value->ToString());
	}
}
