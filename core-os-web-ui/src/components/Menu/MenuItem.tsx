import {
    PageDescriptor,
    useIsCurrentPage,
    useIsNavigationBlocked,
    useNavigateToPage
} from "../../contexts/NavigationContext";
import {useMemo} from "preact/hooks";

type MenuItemProps = {
    pageDescriptor: PageDescriptor
}

export default function MenuItem({pageDescriptor}: MenuItemProps) {

    const navigateToPage = useNavigateToPage(pageDescriptor?.name);
    const isCurrentPage = useIsCurrentPage(pageDescriptor?.name);
    const isNavigationBlocked = useIsNavigationBlocked();

    const classNames = useMemo(() => {

        let result: string = '';

        if (isCurrentPage) {
            result = 'pl-button full menu__item active';
        } else {
            result = 'pl-button full menu__item';
        }

        if (isNavigationBlocked) {
            result += ' disabled';
        }

        return result;

    }, [isNavigationBlocked, isCurrentPage]);

    if (!pageDescriptor.isVisible()) {
        return (<></>);
    }

    return (
        <button className={classNames} onClick={navigateToPage}>{pageDescriptor.displayName}</button>
    )

}