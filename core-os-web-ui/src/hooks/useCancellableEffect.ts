import {useEffect} from "preact/hooks";

export default function useAsyncEffect<T>(clientPromise: () => Promise<T>, asyncCallback: (result: T) => void) {

    useEffect(() => {

        let rejectFunction;

        const cancelPromise = new Promise((_, reject) => {
            rejectFunction = reject;
        });

        Promise.race([clientPromise(), cancelPromise]).then(result => {
            asyncCallback(result as T);
        }).catch(err => {
            // Ignore cancellation errors
        });

        return () => {
            rejectFunction();
        };

    }, [clientPromise, asyncCallback]);

}