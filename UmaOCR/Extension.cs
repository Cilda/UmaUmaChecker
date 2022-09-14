using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;

namespace UmaOCRDll
{
    public static class Extension
    {
        public static Task<T> AsTask<T>(this IAsyncOperation<T> operation)
        {
            var tcs = new TaskCompletionSource<T>();

            operation.Completed = delegate
            {
                switch (operation.Status)
                {
                    case AsyncStatus.Completed:
                        tcs.SetResult(operation.GetResults());
                        break;
                    case AsyncStatus.Error:
                        tcs.SetException(operation.ErrorCode);
                        break;
                    case AsyncStatus.Canceled:
                        tcs.SetCanceled();
                        break;
                }
            };

            return tcs.Task;
        }

        public static TaskAwaiter<T> GetAwaiter<T>(this IAsyncOperation<T> operation)
        {
            return operation.AsTask().GetAwaiter();
        }
    }
}
