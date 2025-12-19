import {useCallback, useState} from "preact/hooks";

export function useLoadingState() {

    const [loadingCounter, setLoadingCounter] = useState<number>(0);

    const inc = useCallback(() => {
        setLoadingCounter((count) => count + 1);
    }, []);

    const dec = useCallback(() => {
        setLoadingCounter((count) => Math.max(0, count - 1));
    }, []);

    const isLoading = loadingCounter > 0;
    return [isLoading, inc, dec] as const;
}