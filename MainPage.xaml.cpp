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

// Raw samples converted into static byte arrays
#include "sintel_audio_clear_bin.h"
#include "sintel_audio_encrypted_bin.h"

// manifest urls for both assets
// http://playready.directtaps.net/media/hls/sintelh264aac/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)
// http://playready.directtaps.net/media/hls/sintelh264aacenc/sintel-1280-surround-m3u8-aapl.ism/manifest(format=m3u8-aapl)

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

	// as stated in the documentation, for a MediaStreamSource, the MediaProtectionManager must be set
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
