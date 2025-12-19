import './app.css'
import MainLayout from './components/MainLayout/MainLayout'
import {NavigationProvider, usePageRenderer} from "./contexts/NavigationContext";
import useBootstrap from "./hooks/useBootstrap";

export function App() {

    useBootstrap();

    return (<NavigationProvider>
            <RootRenderer/>
        </NavigationProvider>
    )
}

export function RootRenderer() {

    const pageRenderer = usePageRenderer();

    return (<MainLayout>
        {pageRenderer}
    </MainLayout>)
}
