import {useCallback, useContext, useState} from "preact/hooks";
import HomePage from "../pages/HomePage";
import {createContext, JSX} from "preact";
import useAsyncEffect from "../hooks/useCancellableEffect";
import {SurfaceService} from "../api/SurfaceService";
import SurfaceUIPage from "../pages/SurfaceUIPage";

export interface NavigationPage {
    name: string
    displayName: string
    createRenderer: JSX.Element
    isVisible?: () => boolean
}

interface NavigationContext {
    allPages: NavigationPage[]
    currentPage: NavigationPage
    changePage: (page: NavigationPage) => void
    blockNavigation: () => () => void;
    navigationBlockedCount: number;
}

const pages: NavigationPage[] = [
    {name: 'home', displayName: 'Home', createRenderer: <HomePage/>}
]

const context = createContext<NavigationContext>({
    allPages: pages,
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

        navigationContext.changePage(navigationContext.allPages.find(page => page.name === pageName) || navigationContext.allPages[0]);

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

    const navigationContext = useContext(context);
    return navigationContext.allPages.map(page => ({name: page.name, displayName: page.displayName, isVisible: page.isVisible || (() => true)}));
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
            allPages: prevState.allPages,
            currentPage: prevState.currentPage,
            changePage: prevState.changePage,
            blockNavigation: prevState.blockNavigation,
            navigationBlockedCount: prevState.navigationBlockedCount + 1
        }));

        return () => {
            setContextValue((prevState) => ({
                allPages: prevState.allPages,
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
                allPages: prevState.allPages,
                currentPage: page,
                changePage: prevState.changePage,
                blockNavigation: prevState.blockNavigation,
                navigationBlockedCount: prevState.navigationBlockedCount
            }

        });
    }, []);

    const loadDynamicPages = useCallback(async () => {

        const allPages = [...pages];
        const surfaces = await SurfaceService.getSurfaces();

        surfaces?.surfaces.forEach(surface => {
            allPages.push({
                name: `surface-${surface.surfaceName}`,
                displayName: surface.displayName,
                createRenderer: <SurfaceUIPage surface={surface}></SurfaceUIPage>
            });
        });

        return allPages;
    }, []);

    const updatePages = useCallback((allPages: NavigationPage[]) => {

        setContextValue((prevState) => {

            if (prevState.navigationBlockedCount > 0) {
                return prevState;
            }

            // Find the current page in the new list of pages, if it doesn't exist, fallback to the first page
            const currentPage = allPages.find(page => page.name === prevState.currentPage.name) || allPages[0];

            return {
                allPages: allPages,
                currentPage: currentPage,
                changePage: prevState.changePage,
                blockNavigation: prevState.blockNavigation,
                navigationBlockedCount: prevState.navigationBlockedCount
            }

        });

    }, []);

    useAsyncEffect(loadDynamicPages, updatePages);

    const [contextValue, setContextValue] = useState<NavigationContext>({
        allPages: pages,
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

