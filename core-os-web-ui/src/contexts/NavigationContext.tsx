import {useCallback, useContext, useMemo, useState} from "preact/hooks";
import HomePage from "../pages/HomePage";
import {createContext, JSX} from "preact";

export interface NavigationPage {
    name: string
    displayName: string
    createRenderer: JSX.Element
    isVisible?: () => boolean
}

interface NavigationContext {
    currentPage: NavigationPage
    changePage: (page: NavigationPage) => void
    blockNavigation: () => () => void;
    navigationBlockedCount: number;
}

const pages: NavigationPage[] = [
    {name: 'home', displayName: 'Home', createRenderer: <HomePage/>}
]

const context = createContext<NavigationContext>({
    currentPage: pages[0],
    changePage: () => {
    },
    blockNavigation: () => {
        return () => {
        }
    },
    navigationBlockedCount: 0
});

export interface PageDescriptor {
    name: string
    displayName: string
    isVisible: () => boolean
}

export function useNavigateToPage(pageName: string) {

    const navigationContext = useContext(context);

    const changePage = useCallback(() => {

        navigationContext.changePage(pages.find(page => page.name === pageName) || pages[0]);

    }, [pageName]);

    return changePage;
}

export function useIsCurrentPage(pageName: string) {

    const navigationContext = useContext(context);
    return navigationContext.currentPage.name === pageName;
}

export function usePageRenderer() {

    const navigationContext = useContext(context);
    return navigationContext.currentPage.createRenderer;
}

export function useGetPageDescriptors() {

    return useMemo(() => {
        return pages.map(page => ({name: page.name, displayName: page.displayName, isVisible: page.isVisible || (() => true)}));
    }, []);
}

export function useIsNavigationBlocked() {

    const navigationContext = useContext(context);
    return navigationContext.navigationBlockedCount > 0;
}

export function useBlockNavigation() {

    const navigationContext = useContext(context);
    return navigationContext.blockNavigation;
}

export function NavigationProvider({children}: { children: any }) {

    const blockNavigation = useCallback(() => {

        setContextValue((prevState) => ({
            currentPage: prevState.currentPage,
            changePage: prevState.changePage,
            blockNavigation: prevState.blockNavigation,
            navigationBlockedCount: prevState.navigationBlockedCount + 1
        }));

        return () => {
            setContextValue((prevState) => ({
                currentPage: prevState.currentPage,
                changePage: prevState.changePage,
                blockNavigation: prevState.blockNavigation,
                navigationBlockedCount: prevState.navigationBlockedCount - 1
            }));
        }

    }, []);

    const changePage = useCallback((page: NavigationPage) => {
        setContextValue((prevState) => {

            if (prevState.navigationBlockedCount > 0) {
                return prevState;
            }

            return {
                currentPage: page,
                changePage: prevState.changePage,
                blockNavigation: prevState.blockNavigation,
                navigationBlockedCount: prevState.navigationBlockedCount
            }

        });
    }, []);

    const [contextValue, setContextValue] = useState<NavigationContext>({
        currentPage: pages[0],
        changePage: changePage,
        blockNavigation: blockNavigation,
        navigationBlockedCount: 0
    });

    return (
        <context.Provider value={contextValue}>
            {children}
        </context.Provider>
    )
}

