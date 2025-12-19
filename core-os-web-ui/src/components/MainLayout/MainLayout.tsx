import Menu from '../Menu/Menu'
import './MainLayout.css'

type MainLayoutProps = {
    children?: any
}

export default function MainLayout({children}: MainLayoutProps) {
    return (<div class='main-layout'>
        <div class='main-layout__menu'>
            <Menu/>
        </div> 
        <div class='main-layout__content'>
            {children}
        </div>
    </div>)
}