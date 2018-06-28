//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

struct AU;

namespace MediaStreamSource
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		Windows::Media::Playback::MediaPlayer ^m_mediaPlayer;
		Windows::Foundation::EventRegistrationToken m_mediaOpenedToken;
		Windows::Foundation::EventRegistrationToken m_mediaFailedToken;
		Windows::Media::Protection::MediaProtectionManager ^m_mediaProtectionManager;
		unsigned char *m_buffer;
		unsigned int   m_bufferSize;
		AU            *m_currentAU;

		// MediaPlayer
		void MediaOpened(Windows::Media::Playback::MediaPlayer^ mediaPlayer, Platform::Object^ args);
		void MediaFailed(Windows::Media::Playback::MediaPlayer^ mediaPlayer, Windows::Media::Playback::MediaPlayerFailedEventArgs^ args);

		// MediaStreamSource
		void Starting(Windows::Media::Core::MediaStreamSource ^source, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args);
		void SampleRequested(Windows::Media::Core::MediaStreamSource ^source, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);

		// MediaProtectionManager
		void ServiceRequested(Windows::Media::Protection::MediaProtectionManager ^manager, Windows::Media::Protection::ServiceRequestedEventArgs ^args);
		void ComponentLoadFailed(Windows::Media::Protection::MediaProtectionManager ^manager, Windows::Media::Protection::ComponentLoadFailedEventArgs ^args);
		void RebootNeeded(Windows::Media::Protection::MediaProtectionManager ^manager);

		// tools
		void CreateMediaPlayer();
		void DestroyMediaPlayer();
		void FixAddresses(unsigned char *buffer, unsigned int length);
		void Log(Platform::String ^str);
		void DumpProperties(Windows::Media::MediaProperties::MediaPropertySet ^properties);

		// UI events
		void PlayClearSamples(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayEncryptedSamples(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayClearURL(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayEncryptedURL(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Stop(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
