using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Windows.Graphics.Imaging;
using Windows.Media.Ocr;
using Windows.Storage.Streams;
using BitmapDecoder = Windows.Graphics.Imaging.BitmapDecoder;
using BitmapEncoder = Windows.Graphics.Imaging.BitmapEncoder;
using BitmapFrame = System.Windows.Media.Imaging.BitmapFrame;

namespace UmaOCRDll
{
    public class OCR
    {
        private static OcrEngine ocrEngine = null;

        public static void Init()
        {
            if (ocrEngine != null) return;

            ocrEngine = OcrEngine.TryCreateFromLanguage(new Windows.Globalization.Language("ja"));
        }

        public static string RecognizeTextFromGrayImage(int width, int height, IntPtr pixels, int size, int stride)
        {
            BitmapSource src = BitmapSource.Create(width, height, 96, 96, PixelFormats.Gray8, null, pixels, size, stride);

            var encoder = new BmpBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(src));

            using (var stream = new MemoryStream())
            {
                encoder.Save(stream);
                using (var softwareBitmap = LoadImageFromByteArrayAsync(stream.ToArray()).GetAwaiter().GetResult())
                {
                    var result = ocrEngine.RecognizeAsync(softwareBitmap).GetAwaiter().GetResult();
                    return result.Text;
                }
            }
        }

        public static string RecognizeText(int width, int height, IntPtr pixels, int size, int stride)
        {
            BitmapSource src = BitmapSource.Create(width, height, 96, 96, PixelFormats.Bgr24, null, pixels, size, stride);

            var encoder = new BmpBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(src));

            using (var stream = new MemoryStream())
            {
                encoder.Save(stream);
                using (var softwareBitmap = LoadImageFromByteArrayAsync(stream.ToArray()).GetAwaiter().GetResult())
                {
                    var result = ocrEngine.RecognizeAsync(softwareBitmap).GetAwaiter().GetResult();
                    return result.Text;
                }
            }
        }

        private static async Task<SoftwareBitmap> LoadImageFromByteArrayAsync(byte[] byteArray)
        {
            using (var randomAccessStream = new InMemoryRandomAccessStream())
            using (var outputStream = randomAccessStream.GetOutputStreamAt(0))
            using (var dataWriter = new DataWriter(outputStream))
            {
                dataWriter.WriteBytes(byteArray);
                await dataWriter.StoreAsync();

                if (!await outputStream.FlushAsync()) throw new InvalidOperationException();

                var decoder = await BitmapDecoder.CreateAsync(randomAccessStream);

                return await decoder.GetSoftwareBitmapAsync(BitmapPixelFormat.Rgba8, BitmapAlphaMode.Premultiplied);
            }
        }
    }
}
